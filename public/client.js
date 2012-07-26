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
    self.vertex_buffer.numItems = 3;
  }
      
  function getShader(gl, id) {
      var shaderScript = document.getElementById(id);

      var str = "";
      var k = shaderScript.firstChild;
      while (k) {
          if (k.nodeType == 3) {
              str += k.textContent;
          }
          k = k.nextSibling;
      }

      var shader_types = {
        'x-shader/x-fragment' : gl.FRAGMENT_SHADER,
        'x-shader/x-vertex'   : gl.VERTEX_SHADER
      }

      var shader = gl.createShader(shader_types[shaderScript.type]);

      gl.shaderSource(shader, str);
      gl.compileShader(shader);

      if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
          alert(gl.getShaderInfoLog(shader));
          return null;
      }

      return shader;
  }
  
  function initShaders(gl) {
      var fragmentShader = getShader(gl, "shader-fs");
      var vertexShader   = getShader(gl, "shader-vs");

      var shaderProgram = gl.createProgram();
      gl.attachShader(shaderProgram, vertexShader);
      gl.attachShader(shaderProgram, fragmentShader);
      gl.linkProgram(shaderProgram);

      if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
          alert("Could not initialise shaders");
      }

      gl.useProgram(shaderProgram);

      shaderProgram.vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
      gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);

      shaderProgram.pMatrixUniform  = gl.getUniformLocation(shaderProgram, "uPMatrix");
      shaderProgram.mvMatrixUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");
      
      return shaderProgram;
  }
  
  var init_webgl = function(canvas) {
    var webgl = canvas.getContext("experimental-webgl");
    console.log(webgl);
    if (!webgl) alert('Could not initialise WebGL');
    webgl.viewportWidth = canvas.width;
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
      gl.uniformMatrix4fv(shaderProgram.pMatrixUniform, false, pMatrix);
      gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
  }
  
  var mouse_x = 0;
  var mouse_y = 0;
  
  
  
  var render = function(canvas, context, gl) {

    var w = canvas.width;
    var h = canvas.height;
    
    // Clear
    canvas.width = w;
    
    gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    mat4.perspective(45, gl.viewportWidth / gl.viewportHeight, 0.1, 100.0, pMatrix);
    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, [-1.5, 0.0, -7.0]);
    
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
    //game.webkitRequestPointerLock();

  });

  console.log('woot');
});
