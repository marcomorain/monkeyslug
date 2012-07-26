$(function() {
  "use strict";

  var triangleVertexPositionBuffer;
  var squareVertexPositionBuffer;
  
  
  
  function initBuffers(gl) {
    triangleVertexPositionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, triangleVertexPositionBuffer);
    var vertices = [
         0.0,  1.0,  0.0,
        -1.0, -1.0,  0.0,
         1.0, -1.0,  0.0
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    triangleVertexPositionBuffer.itemSize = 3;
    triangleVertexPositionBuffer.numItems = 3;

    squareVertexPositionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, squareVertexPositionBuffer);
    vertices = [
         1.0,  1.0,  0.0,
        -1.0,  1.0,  0.0,
         1.0, -1.0,  0.0,
        -1.0, -1.0,  0.0
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    squareVertexPositionBuffer.itemSize = 3;
    squareVertexPositionBuffer.numItems = 4;
  }
      
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
  initBuffers(webgl);

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
    gl.bindBuffer(gl.ARRAY_BUFFER, triangleVertexPositionBuffer);
    gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, triangleVertexPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);
    setMatrixUniforms(gl);
    gl.drawArrays(gl.TRIANGLES, 0, triangleVertexPositionBuffer.numItems);


    mat4.translate(mvMatrix, [3.0, 0.0, 0.0]);
    gl.bindBuffer(gl.ARRAY_BUFFER, squareVertexPositionBuffer);
    gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, squareVertexPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);
    setMatrixUniforms(gl);
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, squareVertexPositionBuffer.numItems);
    
    /*
    
    context.fillStyle = "rgb(200,0,0)";
    context.fillRect (10, 10, 55, 50);
    context.fillStyle = "rgba(0, 0, 200, 0.5)";
    context.fillRect (30, 30, 55, 50);
    
    context.strokeStyle = "rgb(0,0,0,1)";
    context.beginPath();
    context.moveTo(w/2, h/2);
    context.lineTo((w/2) + mouse_x, (h/2) + mouse_y);
    context.stroke();
    
    */
  }

  var update = function(dt) {
    render(game, context, webgl);
    mouse_x = 0;
    mouse_y = 0;
    window.webkitRequestAnimationFrame(update, game);
  };
  
  $game.bind('webkitfullscreenchange', function(e) {
    if (document.webkitFullscreenElement){
//      $game.attr('width', document.width);
//      $game.attr('height', document.height);
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
