/* Armored Fist web worker: stores the input SharedArrayBuffer (sent right after the worker is created,
 * BEFORE the async module init finishes) on self.__fin, then AUTO-STARTS the engine in
 * onRuntimeInitialized (the flow that reliably reaches the menu).  The engine runs BLOCKING, streaming
 * frames out via postMessage and polling self.__fin for live mouse/keyboard each frame. */
self.onerror = (msg, src, line) => { postMessage({ t:'err', m:'worker onerror: '+msg+' @'+src+':'+line }); return false; };
self.onmessage = (e) => {
  if (e.data && e.data.t === 'sab' && e.data.sab && e.data.sab.byteLength){
    self.__fin = new Int32Array(e.data.sab); self.__mseq = -1;
    postMessage({ t:'log', m:'worker: input SAB attached' });
  }
};
try {
  self.importScripts('fist.js');
  FistModule({
    print:    (t) => postMessage({ t:'log', m:'[out] '+t }),
    printErr: (t) => postMessage({ t:'log', m:'[err] '+t }),
    preRun: [ function(Module){
      const E = Module.ENV || (Module.ENV = {});
      E.FIST_DATADIR='armoredfist'; E.FIST_OPL='1'; E.FIST_SB='1'; E.FIST_TICK_HZ='1000';
    } ],
    onRuntimeInitialized: function(){
      postMessage({ t:'ready' });
      try { this.ccall('fist_web_start', null, [], []); }
      catch(e){ postMessage({ t:'err', m:'engine threw: '+(e&&e.message||e) }); }
      postMessage({ t:'exited' });
    }
  }).then(m=>{}, e=>postMessage({ t:'err', m:'module rejected: '+(e&&e.message||e) }));
} catch(e){ postMessage({ t:'err', m:'worker top-level threw: '+(e&&e.message||e) }); }
