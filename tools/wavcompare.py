#!/usr/bin/env python3
"""wavcompare.py -- audio-verify metric for the port's PCM vs the DOSBox oracle WAV.

The framebuffer analog is `compare -metric AE`.  Audio needs a resample-tolerant metric because the two
sides differ in mixer sample rate (the port emits PCM at the DSP-programmed rate; DOSBox resamples the
SB stream to 44100 stereo).  This mono-mixes both, resamples to a common rate, aligns by best
cross-correlation lag, and reports:
  - peak/RMS of each
  - normalized cross-correlation at best lag (1.0 = identical waveform)
  - per-sample AE at an amplitude tolerance (the eventual bit-exact gate: AE==0 at tol 0)

Usage: tools/wavcompare.py A.wav B.wav [--rate 22050] [--tol 0]
"""
import sys, wave, array, argparse, math

def load(path):
    w = wave.open(path, 'rb'); ch=w.getnchannels(); sw=w.getsampwidth(); sr=w.getframerate()
    raw = w.readframes(w.getnframes())
    if sw==2: a=array.array('h'); a.frombytes(raw)
    elif sw==1: a=array.array('h',[ (b-128)<<8 for b in raw ])
    else: raise SystemExit("unsupported width %d"%sw)
    if ch==2: a=[ (a[i]+a[i+1])//2 for i in range(0,len(a),2) ]
    else: a=list(a)
    return a, sr

def resample(a, sr, dst):
    if sr==dst or not a: return a
    n=int(len(a)*dst/sr); out=[0]*n
    for i in range(n):
        x=i*sr/dst; j=int(x); f=x-j
        out[i]= a[j] if j+1>=len(a) else int(a[j]*(1-f)+a[j+1]*f)
    return out

def rms(a): return int((sum(x*x for x in a)/len(a))**0.5) if a else 0

def bestlag(a,b,maxlag):
    # coarse cross-correlation over a downsampled window
    la=min(len(a),len(b));
    if la<1000: return 0,0.0
    step=max(1,la//4000)
    aa=a[:la:step]; bb=b[:la:step]; ml=maxlag//step
    best=(0,-2.0)
    na=math.sqrt(sum(x*x for x in aa)) or 1
    for lag in range(-ml,ml+1):
        s=0.0; nb=0.0
        for i in range(len(aa)):
            j=i+lag
            if 0<=j<len(bb): s+=aa[i]*bb[j]; nb+=bb[j]*bb[j]
        c=s/(na*(math.sqrt(nb) or 1))
        if c>best[1]: best=(lag*step,c)
    return best

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('a'); ap.add_argument('b')
    ap.add_argument('--rate',type=int,default=22050); ap.add_argument('--tol',type=int,default=64)
    o=ap.parse_args()
    a,sra=load(o.a); b,srb=load(o.b)
    a=resample(a,sra,o.rate); b=resample(b,srb,o.rate)
    print("A %-40s sr=%d peak=%d rms=%d n=%d"%(o.a,sra,max(map(abs,a)) if a else 0,rms(a),len(a)))
    print("B %-40s sr=%d peak=%d rms=%d n=%d"%(o.b,srb,max(map(abs,b)) if b else 0,rms(b),len(b)))
    lag,corr=bestlag(a,b,o.rate//4)
    print("best lag=%d samples (%.1f ms)  normalized xcorr=%.4f"%(lag,1000*lag/o.rate,corr))
    # per-sample AE at best lag over the overlap
    if lag>=0: aa,bb=a[lag:],b
    else: aa,bb=a,b[-lag:]
    m=min(len(aa),len(bb)); ae=sum(1 for i in range(m) if abs(aa[i]-bb[i])>o.tol)
    print("per-sample AE(|d|>%d)= %d / %d (%.2f%%)"%(o.tol,ae,m,100*ae/m if m else 0))
    print("VERDICT:", "BIT-EXACT" if ae==0 else ("STRONG MATCH" if corr>0.95 else ("PARTIAL" if corr>0.5 else "MISMATCH")))

if __name__=='__main__': main()
