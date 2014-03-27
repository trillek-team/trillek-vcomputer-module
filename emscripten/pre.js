
// Trace function to console
function errtrace( msg ) {
  if (window.console) {
    console.log('ERR> ' +  msg);
  } else if ( typeof( jsTrace ) != 'undefined' ) {
    jsTrace.send('ERR> ' +  msg );
  }
};

function ptrace( msg ) {
  if (window.console) {
    console.log(msg);
  } else if ( typeof( jsTrace ) != 'undefined' ) {
    jsTrace.send( msg );
  }
};

var Module = {
  'print' : ptrace,
  'printerr' : errtrace,
};

