$(function() {
  "use strict";

  var url = $.url();
  var map = url.param('map') || 'start';

  var uniforms = {
    model_view_matrix: mat4.create(),
    projection_matrix: mat4.create()
    // normal
  };
  
  var mouse_x = 0;
  var mouse_y = 0;
  
  var ambient_light = vec3.create([0.1, 0.1, 0.1]);

  var player = vec3.create([0, -30, 0]);
  var yaw = 0;
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

      var success = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
      
      if (!success) {
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

      var renderer = {
        program    : program,
        attributes : {},
        uniforms   : {}
      };


      var set_vertex_attribute = function(renderer, name){
        var attribute = gl.getAttribLocation(renderer.program, name);
        gl.enableVertexAttribArray(attribute);
        renderer.attributes[name] = attribute;
      };

      var set_uniform_attribute = function(renderer, name){
        renderer.uniforms[name] = gl.getUniformLocation(renderer.program, name);
      };

      set_vertex_attribute(renderer, 'position');
      //set_vertex_attribute(renderer, 'color');
      //set_vertex_attribute(renderer, 'normal');

      set_uniform_attribute(renderer, 'projection_matrix');
      set_uniform_attribute(renderer, 'model_view_matrix');

      return renderer;
  }
  
  var init_webgl = function(canvas) {
    var gl = canvas.getContext("experimental-webgl");
    if (!gl) console.log('Could not initialise WebGL');    
    gl.clearColor(0.7, 0.7, 0.9, 1);
    //gl.enable(gl.DEPTH_TEST);
    //gl.cullFace(gl.FRONT);
    //gl.enable(gl.CULL_FACE)
    
    return gl;
  };
    
  
  var game = $('#game')[0];
  var $game = $(game);
  var gl = init_webgl(game);
  
  var renderer = initShaders(gl);
  
  var triangle = null;
  
  var lights = [];
  
  var to_vertex = function(data) {
    return vec3.create(_.map(data.split(' '), parseFloat));
  };
  
  var to_angle = function(data) {
    return parseFloat(data) * Math.PI / 180.0;
  };

  $.getJSON('start.bsp.processed.json').success(function(data){
    console.log(data);

    var level = data.models[0];
    console.log(level);

    var model = {};
    var i,j;

    window.data = data;

    var indices = [];

    for (i=level.face_id; i<level.face_num; i++){
      var face = data.faces[i];

      for (j=face.ledge_id; j<face.ledge_num; j++){
        var ledge = data.ledges[j];
        var edge;
        if (ledge > 0){
          edge = data.edges[ledge];
        } else {
          edge = data.edges[-ledge];
        }
        indices.push(edge[0]);
        indices.push(edge[1]);
      }
    }
    console.log(indices);

    //triangle = new Buffer(gl, _.flatten(data.vertices), indices);

     var vertices = [
        // Front face
         0.0,  10.0,  0.0,
        -10.0, -10.0,  10.0,
         10.0, -10.0,  10.0,
        // Right face
         0.0,  10.0,  0.0,
         10.0, -10.0,  10.0,
         10.0, -10.0, -10.0,
        // Back face
         0.0,  10.0,  0.0,
         10.0, -10.0, -10.0,
        -10.0, -10.0, -10.0,
        // Left face
         0.0,  10.0,  0.0,
        -10.0, -10.0, -10.0,
        -10.0, -10.0,  10.0
    ];


    var indices = [
    0,1,2,
    3,4,5,
    6,7,8,
    9,10,11
    ];


    triangle = new Buffer(gl, vertices, indices);


  });
  
  /*
  $.when( $.getJSON(map + '.bsp.vertices.json'),
          $.getJSON(map + '.bsp.indices.json'),
          $.getJSON(map + '.bsp.entities.json')
          ).done(function(vertices, indices, entities){
            
      var start = _.find(entities[0], function(e){
        return e.classname == 'info_player_start';
      });
      
      // Load the players start position from the map.
      player = to_vertex(start.origin);
      yaw = to_angle(start.angle);      
      
      // E1M1:
      // "origin" "480 -352 88"
      // "angle" "90"
      // hack to be: [-480, 352, -88]);
      // Angle: -Math.PI/2;
      player = vec3.multiply(player, [-1, -1, -1]);
      yaw = -yaw;
      
      _.each(_.filter(entities[0], function(entity){
        return (entity.classname === 'light')
      }), function(light, i) {
        
        if (i<3) console.log(light);
        light.light = parseFloat(light.light || '200');
        light.origin = to_vertex(light.origin);
        lights.push(light);
      });
      
      //console.log(lights);
         
      triangle = new Buffer(gl, vertices[0].vertices, indices[0].indices);    
  }).fail(function(){
    console.log("JSON download error");
  });
*/
  
  function setMatrixUniforms(renderer, gl) {
    _.each(_.keys(uniforms), function(uniform) { 
      gl.uniformMatrix4fv(renderer.uniforms[uniform],  false, uniforms[uniform]);
    });
  }

  var clamp = function(n, min, max) {
    return Math.max(min, Math.min(n, max));
  }
  
  var render = function(canvas, gl, renderer) {

    var w = canvas.width;
    var h = canvas.height;
  
    gl.viewport(0, 0, w, h);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    mat4.perspective(fov, w / h, near_clip, far_clip, uniforms.projection_matrix);
    mat4.identity(uniforms.model_view_matrix);

    yaw   += mouse_sensitivity * mouse_x;
    pitch -= mouse_sensitivity * mouse_y;
    
    pitch = clamp(pitch, pitch_min, pitch_max);
 
    // Quake
    //var nintey = Math.PI / 2;
   // mat4.rotate(uniforms.model_view_matrix, -nintey, [1, 0, 0]);	    // put Z going up
    //mat4.rotate(uniforms.model_view_matrix,  nintey, [0, 0, 1]);	    // put Z going up

    // change to call rotateX,Y,Z
    mat4.rotate(uniforms.model_view_matrix, roll,    [1, 0, 0]);
    mat4.rotate(uniforms.model_view_matrix, pitch,   [0, 1, 0]);
    mat4.rotate(uniforms.model_view_matrix, yaw,     [0, 0, 1]);
    
    var vec_walk   = vec3.scale(vec3.create([-Math.cos(yaw),  Math.sin(yaw), 0]), speed);
    var vec_strafe = vec3.scale(vec3.create([-Math.sin(yaw),  -Math.cos(yaw), 0]), speed);

    if (keys[up])
    {
      player = vec3.add(player, vec_walk);
      
      /*
      _.each(_.first(lights, 16), function(light){
        console.log(vec3.dist(light.origin, player));
        
      });
      */
      
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


    mat4.translate(uniforms.model_view_matrix, player);
    //mat4.inverse(uniforms.model_view_matrix, normalMatrix);
    //mat4.transpose(normalMatrix);
    
    setMatrixUniforms(renderer, gl);

    if (triangle) {

      //var stride = (3 * 4 * 4);
      var stride = 0;
      
      gl.bindBuffer(gl.ARRAY_BUFFER, triangle.vertex_buffer);
      gl.vertexAttribPointer(renderer.attributes['position'],  3, gl.FLOAT, false, stride, 0);
      //gl.vertexAttribPointer(renderer.attributes['normal'],  3, gl.FLOAT, false, stride, 12);
      //gl.vertexAttribPointer(renderer.attributes['color'],   3, gl.FLOAT, false, stride, 24);
      
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, triangle.index_buffer);

      gl.drawElements(gl.TRIANGLES, triangle.num_elements, gl.UNSIGNED_SHORT, 0);
      
    }
  }

  var update = function(dt) {
    window.webkitRequestAnimationFrame(update, game);
    
    lights = _.sortBy(lights, function(light) {
      return vec3.dist(this, light.origin);
    }, player);
    
    render(game, gl, renderer);
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
