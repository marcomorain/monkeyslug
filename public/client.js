$(function() {
  "use strict";
  
  var Buffer = function(gl, vertices, indices) {
    var self = this;
    self.vertex_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, self.vertex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER,          new Float32Array(vertices), gl.STATIC_DRAW);
    
    self.index_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, self.index_buffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,  new Uint16Array(indices),   gl.STATIC_DRAW);
    
    self.num_elements = indices.length;
  }
      
  function getShader(gl, id) {
      var script = $(id).text();
      
      //console.log(script);

      var shader_types = {
        'x-shader/x-fragment' : gl.FRAGMENT_SHADER,
        'x-shader/x-vertex'   : gl.VERTEX_SHADER
      }

      var shader = gl.createShader(shader_types[$(id).attr('type')]);

      gl.shaderSource(shader, script);
      gl.compileShader(shader);

      if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
          console.log(gl.getShaderInfoLog(shader));
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
          console.log("Could not initialise shaders");
      }

      gl.useProgram(program);

      program.vertexPositionAttribute = gl.getAttribLocation(program, 'aVertexPosition');
      gl.enableVertexAttribArray(program.vertexPositionAttribute);

      program.vertexColorAttribute = gl.getAttribLocation(program, 'aVertexColor');
      gl.enableVertexAttribArray(program.vertexColorAttribute);
            
      program.vertexNormalAttribute = gl.getAttribLocation(program, 'aVertexNormal');
      gl.enableVertexAttribArray(program.vertexNormalAttribute);
      
      program.vertexColorAttribute = gl.getAttribLocation(program, 'aVertexColor');
      gl.enableVertexAttribArray(program.vertexColorAttribute);

      program.pMatrixUniform  = gl.getUniformLocation(program, 'uPMatrix');
      program.mvMatrixUniform = gl.getUniformLocation(program, 'uMVMatrix');
      program.normalMatrix = gl.getUniformLocation(program, 'uNormalMatrix');
      
      return program;
  }
  
  var init_webgl = function(canvas) {
    var gl = canvas.getContext("experimental-webgl");
    if (!gl) console.log('Could not initialise WebGL');    
    gl.clearColor(0.7, 0.7, 0.9, 1);
    gl.enable(gl.DEPTH_TEST);
    gl.cullFace(gl.FRONT);
    gl.enable(gl.CULL_FACE)
    
    return gl;
  };
    
  
  var game = $('#game')[0];
  var $game = $(game);
  var gl = init_webgl(game);
  
  var shaderProgram = initShaders(gl);
  
  
  var triangle = null;
  
  var map = 'e1m1';
  map = 'start';
  
  $.getJSON(map + '.bsp.vertices.json', function(vertices) {
    $.getJSON(map + '.bsp.indices.json', function(indices) {
      triangle = new Buffer(gl, vertices.vertices, indices.indices);
    });
  }).error(function(e) { console.log(e); console.log("error"); })
  
  

  var mvMatrix      = mat4.create();
  var pMatrix       = mat4.create();
  var normalMatrix  = mat4.create();

  function setMatrixUniforms(gl) {
      gl.uniformMatrix4fv(shaderProgram.pMatrixUniform,  false, pMatrix);
      gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
      gl.uniformMatrix4fv(shaderProgram.normalMatrix,    false, normalMatrix);
  }
  
  var mouse_x = 0;
  var mouse_y = 0;

  
  

  // Player position at origin
  // E1M1
  // "classname" "info_player_start"
  // "origin" "480 -352 88"
  // "angle" "90"
  var player = vec3.create([-480, 352, -88]);
  var yaw = -Math.PI/2;
  var roll = 0;
  var pitch = 0;
    
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
  
  var render = function(canvas, gl) {

    var w = canvas.width;
    var h = canvas.height;
    
    // Clear
    canvas.width = w;
    
    gl.viewport(0, 0, w, h);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    mat4.perspective(fov, w / h, near_clip, far_clip, pMatrix);
    mat4.identity(mvMatrix);

    yaw   += mouse_sensitivity * mouse_x;
    pitch -= mouse_sensitivity * mouse_y;
    
    pitch = clamp(pitch, pitch_min, pitch_max);
 
    // Quake
    var nintey = Math.PI / 2;
    mat4.rotate(mvMatrix, -nintey, [1, 0, 0]);	    // put Z going up
    mat4.rotate(mvMatrix,  nintey, [0, 0, 1]);	    // put Z going up

    // change to call rotateX,Y,Z
    mat4.rotate(mvMatrix, roll,    [1, 0, 0]);
    mat4.rotate(mvMatrix, pitch,   [0, 1, 0]);
    mat4.rotate(mvMatrix, yaw,     [0, 0, 1]);
    
    var vec_walk   = vec3.scale(vec3.create([-Math.cos(yaw),  Math.sin(yaw), 0]), speed);
    var vec_strafe = vec3.scale(vec3.create([-Math.sin(yaw),  -Math.cos(yaw), 0]), speed);

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
    mat4.inverse(mvMatrix, normalMatrix);
    mat4.transpose(normalMatrix);
    
    setMatrixUniforms(gl);

    if (triangle) {

      var stride = (3 * 4 * 4);
      
      gl.bindBuffer(gl.ARRAY_BUFFER, triangle.vertex_buffer);
      gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, 3, gl.FLOAT, false, stride, 0);
      gl.vertexAttribPointer(shaderProgram.vertexNormalAttribute,   3, gl.FLOAT, false, stride, 12);
      gl.vertexAttribPointer(shaderProgram.vertexColorAttribute,    3, gl.FLOAT, false, stride, 24);
      
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, triangle.index_buffer);

      gl.drawElements(gl.TRIANGLES, triangle.num_elements, gl.UNSIGNED_SHORT, 0);
      
    }
  }

  var update = function(dt) {
    window.webkitRequestAnimationFrame(update, game);
    render(game, gl);
    mouse_x = 0;
    mouse_y = 0;
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
  
  
  var mouse = false;
  $game.mousedown(function(e){
    mouse = true;
  });
  
  $(document).mouseup(function(e){
    mouse = false;
  });
  
  
  var go_fullscreen = function(){
    game.webkitRequestFullScreen(Element.ALLOW_KEYBOARD_INPUT);
    game.webkitRequestPointerLock();
  };
  
  $game.dblclick(function(e){
    if (!document.webkitFullscreenElement) {
      go_fullscreen();
    }
  });
  
  $game.mousemove(function(e) {
    
    if (!mouse && !document.webkitFullscreenElement) return;
    var orig = e.originalEvent;
    
    var x = orig.webkitMovementX || 0;
    var y = orig.webkitMovementY || 0;
    
    mouse_x += x;
    mouse_y -= y;
  });
  
  window.webkitRequestAnimationFrame(update, game);

  $('#fullscreen').click(function(){
    go_fullscreen();
  });
});
