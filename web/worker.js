/* Armored Fist web worker: runs the engine BLOCKING (fist_web_start never returns), posting a copy of
 * the framebuffer + palette to the main thread each ~frame. Errors are forwarded to the main thread. */
self.onerror = function(msg, src, line, col, err){
  postMessage({ t:'err', m: 'worker onerror: ' + msg + ' @' + src + ':' + line });
  return false;
};
try {
  postMessage({ t:'log', m:'worker: importScripts fist.js' });
  self.importScripts('fist.js');   // defines the FistModule factory (MODULARIZE)
  postMessage({ t:'log', m:'worker: FistModule=' + (typeof FistModule) });
  FistModule({
    print:    (t) => postMessage({ t:'log', m:'[out] ' + t }),
    printErr: (t) => postMessage({ t:'log', m:'[err] ' + t }),
    preRun: [ function(Module){
      var E = Module.ENV || (Module.ENV = {});
      E.FIST_DATADIR = 'armoredfist'; E.FIST_OPL = '1'; E.FIST_SB = '1'; E.FIST_TICK_HZ = '1000';
    } ],
    onRuntimeInitialized: function(){
      postMessage({ t:'ready' });
      try { this.ccall('fist_web_start', null, [], []); }
      catch(e){ postMessage({ t:'err', m:'engine threw: ' + (e && e.message || e) }); }
      postMessage({ t:'exited' });
    }
  }).then(
    ()=> postMessage({ t:'log', m:'worker: module promise resolved' }),
    (e)=> postMessage({ t:'err', m:'module promise rejected: ' + (e && e.message || e) })
  );
} catch(e){
  postMessage({ t:'err', m:'worker top-level threw: ' + (e && e.message || e) });
}
