var prevKey = 0;
var kmod = 0;

+function ($) { "use strict";

  // Trace function to console
  function trace( msg ) {
    if (window.console) {
      console.log(msg);
    } else if ( typeof( jsTrace ) != 'undefined' ) {
      jsTrace.send( msg );
    }
  };

  // Beauty hexadecimal numbers
  function decimalToHex(d, padding) {
    if (d < 0)
      d = 0xFFFFFFFF + d + 1;

    var hex = Number(d).toString(16).toUpperCase();
    padding = typeof (padding) === "undefined" || padding === null ? padding = 8 : padding;

    while (hex.length < padding) {
      hex = "0" + hex;
    }

    return hex;
  }

  function isIE () {
    var myNav = navigator.userAgent.toLowerCase();
    return (myNav.indexOf('msie') != -1) ? parseInt(myNav.split('msie')[1]) : false;
  }

  /**
   * Convert JS KeyCodes from evt.keyCode to TR3200 keyboard codes
   */
  function JSKeyCodeToScanCode (key) {
    switch (key) {
      case 13: // Return
        return 257;

      case 16: // Shift
        return 340;

      case 17: // Control
        return 341;

      case 18:  // Alt
        return 342;

      case 225: // Alt Gr
        return 346;

      case 37: // Left arrow
        return 263;

      case 38: // Up arrow
        return 265;

      case 39: // Right arrow
        return 262;

      case 40: // Down arrow
        return 264;

      case 45: // Insert
        return 260;

      case 46: // Delete
        return 261;

      case 219: // Left Bracket
        return 91;

      case 221: // Right Bracket
        return 93;

      case 222: // Apostrophe (' ")
        return 39;

      case 188: // Comma (, < )
        return 44;

      case 190: // Period (. >)
        return 46;

      // TODO More cases

      default:
        return key;
    }
    return key;
  }

  var vm = new Module.VComputer(128*1024);
  Module.SetTR3200CPU(vm, 100000);
  var tda = new Module.TDADev();
  vm.AddDevice(5, Module.TDADev.ToIDevice(tda));
  var gkey = new Module.GKeyboardDev();
  vm.AddDevice(3, Module.GKeyboardDev.ToIDevice(gkey));

  // Generate a Buffer that bridges ToRGBATexture and WebGL texture
  // Get data byte size, allocate memory on Emscripten heap, and get pointer
  var nBytes = 320*240*4;
  var texturePtr = Module._malloc(nBytes);

  // Creates a Type Array using this emscripten heap memory block
  var textureHeap = new Uint8Array(Module.HEAPU8.buffer, texturePtr, nBytes);
  for (var i=0; i < 320*240*4; i = i + 4) // And fill it of black
  textureHeap.set([0, 0 ,0 ,0], i);



  var canvas;         // Canvas were to write
  var mode2d = false; // We must use canvas 2D ?
  var context;        // Canvas 2D context
  var imageData;

  // WebGL stuff
  var gl;                       // WebGL context

  var shaderProgram;            // Id of linked shader program

  var glTexture;                // Id of WebGL texture were to paint CDA screen

  var VPBuffer;                 // Vertex Position Buffer
  var VTexCoordBuffer;          // Vertex UV coord Buffer
  var VIndexBuffer;             // Vertex Index Buffer

  var mvMatrix = mat4.create(); // Model-View matrix
  var mvMatrixStack = [];
  var pMatrix = mat4.create();  // Proy matrix

  /**
   * Inits WebGL stuff
   */
  function initGL(canvas) {
    gl = canvas.getContext("experimental-webgl");
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    if (!gl) {
      throw("Could not initialise WebGL, sorry :-(");
    }
  }

  // Grabs teh shader and compiles it
  function getShader(gl, id) {
    var shaderScript = document.getElementById(id);
    if (!shaderScript) {
      return null;
    }

    var str = "";
    var k = shaderScript.firstChild;
    while (k) {
      if (k.nodeType == 3) {
        str += k.textContent;
      }
      k = k.nextSibling;
    }

    var shader;
    if (shaderScript.type == "x-shader/x-fragment") {
      shader = gl.createShader(gl.FRAGMENT_SHADER);
    } else if (shaderScript.type == "x-shader/x-vertex") {
      shader = gl.createShader(gl.VERTEX_SHADER);
    } else {
      return null;
    }

    gl.shaderSource(shader, str);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      throw gl.getShaderInfoLog(shader);
      return null;
    }

    return shader;
  }

  // Initialize and load the shaders
  function initShaders() {
    var fragmentShader = getShader(gl, "shader-fs");
    var vertexShader = getShader(gl, "shader-vs");

    shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
      throw "Could not initialise shaders";
    }

    gl.useProgram(shaderProgram);

    shaderProgram.vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
    gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);

    shaderProgram.textureCoordAttribute = gl.getAttribLocation(shaderProgram, "aTextureCoord");
    gl.enableVertexAttribArray(shaderProgram.textureCoordAttribute);

    shaderProgram.pMatrixUniform = gl.getUniformLocation(shaderProgram, "uPMatrix");
    shaderProgram.mvMatrixUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");
    shaderProgram.samplerUniform = gl.getUniformLocation(shaderProgram, "uSampler");
  }

  /**
   * Updated Texture with CDA last state
   * Also In Canvas 2d API redraws it
   */
  function updateTexture(texture, tda) {
    var dump = new Module.TDAScreen();
    tda.DumpScreen(dump);
    if (mode2d) {
      dump.toRGBATexture(texture.rawdata.byteOffset);

      var buf8 = new Uint8ClampedArray(texture.rawdata);
      imageData.data.set(buf8);
      // We paint in a temporal canvas to use canvas scale
      var newCanvas = $("<canvas>")
        .attr("width", 320)
        .attr("height", 240)[0];
      newCanvas.getContext("2d").putImageData(imageData, 0, 0);
      context.drawImage(newCanvas, 0, 0);

    } else {
      gl.bindTexture(gl.TEXTURE_2D, texture);

      dump.toRGBATexture(texture.rawdata.byteOffset);

      gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0 , 320, 240,
                       gl.RGBA, gl.UNSIGNED_BYTE, texture.rawdata);
                       gl.bindTexture(gl.TEXTURE_2D, null);
    }
    tda.DoVSync();
  }

  /**
   * Cleans the texture
   */
  function cleanTexture(texture) {
    for (var i=0; i < 320*240*4; i = i + 4) // Fill screen texture of black
    texture.rawdata.set([0, 0 ,0 ,0], i);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texSubImage2D(gl.TEXTURE_2D, 0,
                     0, 0 , 320, 240,
                     gl.RGBA, gl.UNSIGNED_BYTE, texture.rawdata);
                     gl.bindTexture(gl.TEXTURE_2D, null);
  }


  // Sets the texture parameters and load a initial image
  function handleLoadedTexture(texture) {

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
    // Call function and get result
    var dump = new Module.TDAScreen();
    tda.DumpScreen(dump);
    dump.toRGBATexture(texture.rawdata.byteOffset);

    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 320, 240, 0,
                  gl.RGBA, gl.UNSIGNED_BYTE, texture.rawdata);
                  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
                  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
                  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
                  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
                  gl.bindTexture(gl.TEXTURE_2D, null);
  }

  // Init the texture
  function initTexture() {
    if (mode2d) {
      glTexture = {'rawdata' : textureHeap};
    } else {
      glTexture = gl.createTexture();
      glTexture.rawdata = textureHeap;
      handleLoadedTexture(glTexture)
    }
  }

  function mvPushMatrix() {
    var copy = mat4.create();
    mat4.set(mvMatrix, copy);
    mvMatrixStack.push(copy);
  }

  function mvPopMatrix() {
    if (mvMatrixStack.length == 0) {
      throw "Invalid popMatrix!";
    }
    mvMatrix = mvMatrixStack.pop();
  }

  function setMatrixUniforms() {
    gl.uniformMatrix4fv(shaderProgram.pMatrixUniform, false, pMatrix);
    gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
  }

  // Init Vertex Buffers
  function initBuffers() {
    VPBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VPBuffer);
    var vertices = [
      // Front face
      -3.2, -2.4,  0.0,
      3.2, -2.4,  0.0,
      3.2,  2.4,  0.0,
      -3.2,  2.4,  0.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    VPBuffer.itemSize = 3;
    VPBuffer.numItems = 4;

    VTexCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VTexCoordBuffer);
    var textureCoords = [
      // Front face
      0.0, 0.0,
      1.0, 0.0,
      1.0, 1.0,
      0.0, 1.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoords), gl.STATIC_DRAW);
    VTexCoordBuffer.itemSize = 2;
    VTexCoordBuffer.numItems = 4;

    VIndexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, VIndexBuffer);
    var vertexIndices = [
      0, 1, 2,      0, 2, 3,    // Front face
    ];
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(vertexIndices), gl.STATIC_DRAW);
    VIndexBuffer.itemSize = 1;
    VIndexBuffer.numItems = 6;
  }

  // Update WebGL scene
  function drawSceneGL() {
    gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    mat4.perspective(45, gl.viewportWidth / gl.viewportHeight, 0.1, 100.0, pMatrix);

    mat4.identity(mvMatrix);

    mat4.translate(mvMatrix, [0.0, 0.0, -6.0]);

    gl.bindBuffer(gl.ARRAY_BUFFER, VPBuffer);
    gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, VPBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, VTexCoordBuffer);
    gl.vertexAttribPointer(shaderProgram.textureCoordAttribute, VTexCoordBuffer.itemSize, gl.FLOAT, false, 0, 0);


    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, glTexture);
    gl.uniform1i(shaderProgram.samplerUniform, 0);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, VIndexBuffer);
    setMatrixUniforms();
    gl.drawElements(gl.TRIANGLES, VIndexBuffer.numItems, gl.UNSIGNED_SHORT, 0);

    var error = gl.getError();
    if (error != gl.NO_ERROR && error != gl.CONTEXT_LOST_WEBGL) {
      trace ("WebGL error!");
      //mode2d = true;
    }
  }


  var canvas_init = false;
  // Init al canvas stuff and uses WebGL or 2d mode
  function setupCanvas(canvas) {
    if (!canvas_init) {
      if (! mode2d) {
        try {
          initGL(canvas);
          initShaders();
          initBuffers();
          initTexture();

          gl.clearColor(0.2, 0.2, 0.2, 1.0);
          gl.enable(gl.DEPTH_TEST);

          gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
          gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        } catch (e) {
          trace(e);
          mode2d = true;
        }
      }
      if(mode2d) {
        initTexture();
        context = canvas.getContext('2d');
        context.scale(canvas.width / 320.0, canvas.height / 240.0);
        imageData = context.createImageData(320, 240);

        context.fillStyle="#000000";
        context.fillRect(0,0, canvas.width, canvas.height);
      }
      canvas_init = true;
    }
  }


  // Main code to execute here ************************************************

  // Use to control timings
  var lastTime = new Date().getTime();
  var updTexTime = 40;
  var updSpeed = 3000;

  var cycles = 17500; // How many cycles are being executed in a bath
  var tms = (cycles / 1000000.0) * 1000; // Time in ms that should be running

  var running = false;  // Looping ?
  var step_mode = false; // Step mode ?

  function tick() { // This is executed every tick
    if (running)
      requestAnimFrame(tick);

    var timeNow = new Date().getTime();
    if (lastTime != 0) {
      var elapsed = timeNow - lastTime;

      if (step_mode) {    // Step mode ! **************************************
        step_mode = false;
        var state = new Module.TR3200State();
        Module.GetTR3200State(vm, state);

        $('#pc_ex').text( decimalToHex(state.GetPC()) );
        $('#instr').text( Module.DisassemblyTR3200(vm, state.GetPC()) );

        var ticks = vm.Step(elapsed);

        // Update VM machine state display
        for (var i=0; i <= 11; i++ ) {
          var r = state.GetR(i);
          $('#r' + i.toString() ).text( decimalToHex(r) );
        }

        $('#bp').text( decimalToHex(state.GetR(12)) );
        $('#sp').text( decimalToHex(state.GetR(13)) );
        $('#ia').text( decimalToHex(state.GetR(14)) );
        $('#flags').text( decimalToHex(state.GetR(15)) );

      } else {  // Run mode ! *************************************************
        vm.Tick(cycles, elapsed);
        cycles = (1000000.0 * elapsed * 0.001);
        if (cycles <= 3)
          cycles = 3;

        if (cycles > 90000) // Dirty fix for clock.asm
          cycles = 90000;
      }

      updTexTime += elapsed;
      if (updTexTime >= 40) { // 25 FPS in milliseconds
        updateTexture(glTexture, tda);
        updTexTime -= 40;
      }

      updSpeed += elapsed;
      if (updSpeed >= 3000) {
        var speed = 100.0 * (tms / elapsed)
        $("#cpu_speed").text(speed.toPrecision(4) + " %");
        updSpeed -= 3000;
      }
    }

    if (! mode2d)
      drawSceneGL();

    lastTime = timeNow;
  }


  // Init all
  $('#load_btn').button();
  $('#run_btn').button();
  $('#run_btn').prop('disabled', true);
  $('#step_btn').button();
  $('#step_btn').prop('disabled', true);
  $('#reset_btn').button();
  $('#reset_btn').prop('disabled', true);

  canvas = document.getElementById('canvas1');
  canvas.addEventListener("webglcontextlost", function(evt) {
    evt.preventDefault();
  }, false);

  canvas.addEventListener("webglcontextrestored", function(evt) {
    setupCanvas(canvas);
  }, false);

  if ($('#webgl').prop('checked')) {
    mode2d = false;
  } else {
    mode2d = true;
  }

  // Attach event listeners ***************************************************
  // Run / Stop button
  $('#run_btn').on('click', function (evt) {
    if (running) {
      running = false;
      $('#run_btn').html('<span class="glyphicon glyphicon glyphicon-play"></span> Run');
      $('#reset_btn').prop('disabled', false);
      $('#step_btn').prop('disabled', false);
      $('#load_btn').prop('disabled', false);

    } else {
      running = true;
      tick();
      $('#run_btn').html('<span class="glyphicon glyphicon glyphicon-stop"></span> Stop');
      $('#reset_btn').prop('disabled', true);
      $('#step_btn').prop('disabled', true);
      $('#load_btn').prop('disabled', true);
    }
  });

  // Reset button
  $('#reset_btn').on('click', function (evt) {
    vm.Reset();

    if (!mode2d) {
      cleanTexture(glTexture);
      drawSceneGL();
    } else {
      context.fillStyle="#000000";
      context.fillRect(0,0, canvas.width, canvas.height);
    }
  });

  // Step button
  $('#step_btn').on('click', function (evt) {
    if (! running) {
      step_mode = true;
      tick();
    }
  });

  // File chooser
  var selector = $('#romfile');
  if (isIE()) { // Piece of crap of IE !!!
    // IE suspends timeouts until after the file dialog closes
    selector.click(function(event) {
      setTimeout(function() {
        if(selector.val().length > 0) {
          if (selector[0].files.length > 0) {
            $('#load_btn').prop('disabled', false);
          } else {
            $('#load_btn').prop('disabled', true);
          }
        }
      }, 0);
    });
  } else {
    selector.on('change', function (evt) {
      if (selector[0].files.length > 0) {
        $('#load_btn').prop('disabled', false);
      } else {
        $('#load_btn').prop('disabled', true);
      }
    });
  }
  if (selector[0].files.length > 0) {
    $('#load_btn').prop('disabled', false);
  } else {
    $('#load_btn').prop('disabled', true);
  }

  // Load button
  $('#load_btn').on('click', function (evt) {
    if (! running) { // Not running, we try to load the ROM file
      var selector = $('#romfile');
      // Check for the various File API support.
      if (window.File && window.FileReader && window.FileList && window.Blob) {
        var files = selector[0].files; // FileList object
        var file = files[0];
        var reader = new FileReader();

        reader.onload = (function(theFile){
          return function(e) {

            var bytes = theFile.size;
            if (bytes > 64*1024)
              bytes = 64*1024;

            trace("Loaded ROM file : " + theFile.name + " Size of : " + bytes);
            if (bytes < 1024) {
              $("#rom_size").text(bytes + " Bytes");
            } else {
              $("#rom_size").text((bytes/1024.0).toPrecision(4) + " KiB");
            }

            var filePtr;
            var fileHeap;
            // Get data byte size, allocate memory on Emscripten heap, and get pointer
            filePtr = Module._malloc(bytes);
            // Creates a Type Array using this emscripten heap memory block
            fileHeap = new Uint8Array(Module.HEAPU8.buffer, filePtr, bytes);
            var tmp = new Uint8Array(reader.result, 0, bytes);
            fileHeap.set(tmp);
            vm.SetROM(fileHeap.byteOffset, bytes);
            // Free memory
            //Module._free(fileHeap.byteOffset);

          };
        })(file);

        reader.readAsArrayBuffer(file)
        $("#run_btn").prop('disabled', false);
        $('#step_btn').prop('disabled', false);
        $('#reset_btn').prop('disabled', false);

        setupCanvas(canvas);

        $('#webgl').prop('checked', ! mode2d);
        $('#webgl').prop('disabled', true);

        vm.Reset(); // Enforces reset
      } else {
        trace('The File APIs are not fully supported in this browser.');
      }

    } else {
    }
  });


  // WebGL checkbox
  $('#webgl').on('change', function (evt) {
    if ($('#webgl').prop('checked')) {
      mode2d = false;
    } else {
      mode2d = true;
    }
  });

  // Stores keyboard status
  var keyb = {
    'shift_key' : false,
    'caps_lock' : false,
  };

  // Keyboard events to feed the generic keyboard
  $(document).keydown( function (evt) {
    // read : http://unixpapa.com/js/key.html
    var kCode = evt.charCode || evt.keyCode;
    if (running) {
      trace('down -> ' + kCode);
      prevKey = JSKeyCodeToScanCode(kCode);

      switch (prevKey) { // Keys that not generate a keypress event
        case 340:
        case 344:   // Shift
          gkey.EnforceSendKeyEvent (prevKey, 0x0E, kmod);
          kmod |= 1;
          break;

        case 341:
        case 345:   // Ctrl
          gkey.EnforceSendKeyEvent (prevKey, 0x0F, kmod);
          kmod |= 2;
          break;

        case 342:
        case 346:   // Alt
          gkey.EnforceSendKeyEvent (prevKey, 0x06, kmod);
          kmod |= 3;
          break;

        default:
          break;
      }
    }
    return true;
  });

  $(document).keyup( function (evt) { // We only use this to change kmod value
    var kCode = evt.charCode || evt.keyCode;
    if (running) {
      prevKey = JSKeyCodeToScanCode(kCode);

      switch (prevKey) { // Keys that not generate a keypress event
        case 340:
        case 344:   // Shift
          kmod &= (1 ^0xFFFF);
          break;

        case 341:
        case 345:   // Ctrl
          kmod &= (2 ^0xFFFF);
          break;

        case 342:
        case 346:   // Alt
          kmod &= (3 ^0xFFFF);
          break;

        default:
          break;
      }
    }
    return true;
  });

  $(document).keypress( function (evt) {
    evt.preventDefault();
    if (running) {
      var chr = evt.which & 0xFF; // Ascii / Latin 1
      switch (prevKey) {
        case 257: // Return
          chr = 0x0D;
          break;

        case 262: // Right Arrow
          chr = 0x15;
          break;

        case 263: // Left Arrow
          chr = 0x14;
          break;

        // TODO More cases

        default:
          break;

      }
      trace('scan: '+ prevKey + ' key: ' + chr + ' mod: ' + kmod);
      gkey.EnforceSendKeyEvent (prevKey, chr, kmod);
    }
    return false;
  });

  // Misc
  $('[data-toggle="tooltip"]').tooltip({'placement': 'top'});

}(jQuery);

