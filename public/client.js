$(function() {
  "use strict";
  
  var Buffer = function(gl, vertices) {
    var self = this;
    self.vertex_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, self.vertex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    self.vertex_buffer.itemSize = 3;
    self.vertex_buffer.numItems = vertices.length / 3;
    console.log(self.vertex_buffer);
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
    webgl.clearColor(0.2, 0.2, 0.2, 1.0);
    webgl.enable(webgl.DEPTH_TEST);
    return webgl;
  };
    
  
  var game = $('#game')[0];
  var $game = $(game);
  var context = null; //game.getContext("2d");
  var webgl = init_webgl(game);
  
  var shaderProgram = initShaders(webgl);
  
  
  var triangle = null;
  
  $.getJSON('vertices.json', function(data) {
    triangle = new Buffer(webgl, data.vertices);
  });
  

  var mvMatrix = mat4.create();
  var pMatrix = mat4.create();

  function setMatrixUniforms(gl) {
      gl.uniformMatrix4fv(shaderProgram.pMatrixUniform,  false, pMatrix);
      gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
  }
  
  var mouse_x = 0;
  var mouse_y = 0;

  
  
  var yaw = 0;
  var roll = 0;
  var pitch = 0;

  // Player position at origin
  var player = vec3.create([0,0,0]);
  
  var mouse_sensitivity = 0.01;
  var fov = 45;
  var near_clip = 4;
  var far_clip = 4096;
  var speed = 4;
  
  var pitch_min = -Math.PI/6;
  var pitch_max =  Math.PI/6;
  
  var up    = 87;
  var down  = 83;
  var left  = 65;
  var right = 68;
  var jump  = 32;
  var duck  = 67;
  var keys  = {};
  
  
  var clamp = function(n, min, max) {
    return Math.max(min, Math.min(n, max));
  }
  
  var render = function(canvas, context, gl) {

    var w = canvas.width;
    var h = canvas.height;
    
    // Clear
    canvas.width = w;
    
    gl.viewport(0, 0, w, h);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    mat4.perspective(fov, w / h, near_clip, far_clip, pMatrix);
    mat4.identity(mvMatrix);

    yaw   += mouse_sensitivity * mouse_x;
    pitch += mouse_sensitivity * mouse_y;
    
    pitch = clamp(pitch, pitch_min, pitch_max);
 
    // Quake
    var nintey = Math.PI / 2;
    mat4.rotate(mvMatrix, -nintey, [1, 0, 0]);	    // put Z going up
    mat4.rotate(mvMatrix,  nintey, [0, 0, 1]);	    // put Z going up

    // change to call rotateX,Y,Z
    mat4.rotate(mvMatrix, roll,    [1, 0, 0]);
    mat4.rotate(mvMatrix, pitch,   [0, 1, 0]);
    mat4.rotate(mvMatrix, yaw,     [0, 0, 1]);

    var vec_walk   = vec3.create([0, speed, 0]);
    var vec_strafe = vec3.create([speed, 0, 0]);
    
    vec_walk   = vec3.scale(vec3.create([-Math.cos(yaw),  Math.sin(yaw), 0]), speed);
    vec_strafe = vec3.scale(vec3.create([-Math.sin(yaw),  -Math.cos(yaw), 0]), speed);

    if (keys[up])
    {
      player = vec3.add(player, vec_walk);
    }
    
    if (keys[down])
    {
      player = vec3.subtract(player, vec_walk);
    }
    
    if (keys[left]) 
    {
      player = vec3.add(player, vec_strafe);
    }
    
    if (keys[right]) 
    {
      player = vec3.subtract(player, vec_strafe);
    }
    
    if (keys[jump])
    {
      player = vec3.subtract(player, [0, 0, speed]);
    }
    
    if (keys[duck])
    {
      player = vec3.add(player, [0, 0, speed]);
    }
      
    mat4.translate(mvMatrix, player);
        
    if (triangle) {
      gl.bindBuffer(gl.ARRAY_BUFFER, triangle.vertex_buffer);
      gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, triangle.vertex_buffer.itemSize, gl.FLOAT, false, 0, 0);
      setMatrixUniforms(gl);
      gl.drawArrays(gl.LINE_LOOP, 0,  triangle.vertex_buffer.numItems);
    }
  }

  var update = function(dt) {
    render(game, context, webgl);
    mouse_x = 0;
    mouse_y = 0;
    window.webkitRequestAnimationFrame(update, game);
  };
  
  var original_width  = game.width;
  var original_height = game.height;

  $game.bind('webkitfullscreenchange', function(e) {
    if (document.webkitFullscreenElement){
      game.width = screen.width;
      game.height = screen.height;
    } else {
      game.width = original_width;
      game.height = original_height;
    }
    console.log('fullscreen change');
  });
  
  
  $(document).keydown(function(e) { keys[e.which] = true;  });
  $(document).keyup(  function(e) { keys[e.which] = false; });
  
  $game.mousemove(function(e) {
    
    if (!document.webkitFullscreenElement) return;
    var orig = e.originalEvent;
    
    var x = orig.webkitMovementX || 0;
    var y = orig.webkitMovementY || 0;
    
    mouse_x += x;
    mouse_y += y;
  });    
  
  window.webkitRequestAnimationFrame(update, game);

  $('#fullscreen').click(function(){
    game.webkitRequestFullScreen(Element.ALLOW_KEYBOARD_INPUT);
    game.webkitRequestPointerLock();
  });
});
