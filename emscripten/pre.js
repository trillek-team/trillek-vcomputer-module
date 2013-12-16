
// Trace function to console
var debug_trace = true;
function trace( msg ) {
  if (typeof debug_trace != 'undefined' && debug_trace) {
    if (window.console) {
        console.log(msg);
    } else if ( typeof( jsTrace ) != 'undefined' ) {
        jsTrace.send( msg );
    } else {
        //alert(msg);
    }
  }
};

var Module = {
  'print' : trace,
  'printerr' : trace,
};

