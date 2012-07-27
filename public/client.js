$(function() {
  "use strict";
  
  var Buffer = function(gl) {
    var self = this;
    self.vertex_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, self.vertex_buffer);
    
    var vertices = [
         0.0,  1.0,  0.0,
        -1.0, -1.0,  0.0,
         1.0, -1.0,  0.0
    ];
    
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    
    self.vertex_buffer.itemSize = 3;
    self.vertex_buffer.numItems = vertices.length / 3;
  }
      
  function getShader(gl, id) {
      var script = $(id).text();

      var shader_types = {
        'x-shader/x-fragment' : gl.FRAGMENT_SHADER,
        'x-shader/x-vertex'   : gl.VERTEX_SHADER
      }

      var shader = gl.createShader(shader_types[$(id).attr('type')]);

      gl.shaderSource(shader, script);
      gl.compileShader(shader);

      if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
          alert(gl.getShaderInfoLog(shader));
          return null;
      }

      return shader;
  }
  
  function initShaders(gl) {
      var fragmentShader = getShader(gl, '#shader-fs');
      var vertexShader   = getShader(gl, '#shader-vs');

      var program = gl.createProgram();
      gl.attachShader(program, vertexShader);
      gl.attachShader(program, fragmentShader);
      gl.linkProgram(program);

      if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
          alert("Could not initialise shaders");
      }

      gl.useProgram(program);

      program.vertexPositionAttribute = gl.getAttribLocation(program, 'aVertexPosition');
      gl.enableVertexAttribArray(program.vertexPositionAttribute);

      program.pMatrixUniform  = gl.getUniformLocation(program, 'uPMatrix');
      program.mvMatrixUniform = gl.getUniformLocation(program, 'uMVMatrix');
      
      return program;
  }
  
  var init_webgl = function(canvas) {
    var webgl = canvas.getContext("experimental-webgl");
    if (!webgl) alert('Could not initialise WebGL');
    webgl.viewportWidth  = canvas.width;
    webgl.viewportHeight = canvas.height;
    webgl.clearColor(0.0, 0.0, 0.0, 1.0);
    webgl.enable(webgl.DEPTH_TEST);
    return webgl;
  };
    
  
  var game = $('#game')[0];
  var $game = $(game);
  var context = null; //game.getContext("2d");
  var webgl = init_webgl(game);
  
  var shaderProgram = initShaders(webgl);
  var triangle = new Buffer(webgl);

  var mvMatrix = mat4.create();
  var pMatrix = mat4.create();

  function setMatrixUniforms(gl) {
      gl.uniformMatrix4fv(shaderProgram.pMatrixUniform,  false, pMatrix);
      gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
  }
  
  var mouse_x = 0;
  var mouse_y = 0;
  
  
  var x = -1.5;
  var z = -7;
  
  var render = function(canvas, context, gl) {

    var w = canvas.width;
    var h = canvas.height;
    
    // Clear
    canvas.width = w;
    
    gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    mat4.perspective(45, gl.viewportWidth / gl.viewportHeight, 0.1, 100.0, pMatrix);
    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, [x, 0.0, z]);
    
    x += mouse_x * 0.05;
    z += mouse_y * 0.05;
    
    gl.bindBuffer(gl.ARRAY_BUFFER, triangle.vertex_buffer);
    
    
    gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, triangle.vertex_buffer.itemSize, gl.FLOAT, false, 0, 0);
    setMatrixUniforms(gl);
    gl.drawArrays(gl.LINE_LOOP, 0,  triangle.vertex_buffer.numItems);
  }

  var update = function(dt) {
    render(game, context, webgl);
    mouse_x = 0;
    mouse_y = 0;
    window.webkitRequestAnimationFrame(update, game);
  };
  
  $game.bind('webkitfullscreenchange', function(e) {
    if (document.webkitFullscreenElement){
      // entered fullscreen
    }
    console.log('fullscreen change');
  });
  
  $(game).mousemove(function(e) {
    var orig = e.originalEvent;
    
    var x = orig.webkitMovementX || 0;
    var y = orig.webkitMovementY || 0;
    
    mouse_x += x;
    mouse_y += y;
  });    
  
  window.webkitRequestAnimationFrame(update, game);

  $('#fullscreen').click(function(){
    game.webkitRequestFullScreen();
    game.webkitRequestPointerLock();

  });
});
