$(function() {
  "use strict";
  
  
  var game = $('#game')[0];
  var context = game.getContext("2d");
  
  
  
  var mouse_x = 0;
  var mouse_y = 0;
  
  var render = function(canvas, context) {

    var w = canvas.width;
    var h = canvas.height;
    
    // Clear
    canvas.width = w;
    
    context.fillStyle = "rgb(200,0,0)";
    context.fillRect (10, 10, 55, 50);
    context.fillStyle = "rgba(0, 0, 200, 0.5)";
    context.fillRect (30, 30, 55, 50);
    
    context.strokeStyle = "rgb(0,0,0,1)";
    context.beginPath();
    context.moveTo(w/2, h/2);
    context.lineTo((w/2) + mouse_x, (h/2) + mouse_y);
    context.stroke();
  }

  var update = function(dt) {
    render(game, context);
    mouse_x = 0;
    mouse_y = 0;
    window.webkitRequestAnimationFrame(update, game);
  };
  
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

  console.log('woot');
});
