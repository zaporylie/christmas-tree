window.session = false;

function sendRequest(data, message) {
  data['id'] = window.session;
  socket.emit('method', data);
}

function setMessage(message, status, client) {
  $("#console").append("<div class='c-message c-message-" + status + "'><span class='c-message-user'>" + client + ":</span> " + message + "</div>");
  $("#console").scrollTop($("#console")[0].scrollHeight);
}

function randColors() {
  data = [];
  for (i = 0; i < Math.floor((Math.random() * 10) + 1); i++) {
    data.push(randColor());
  }
  return data;
}

function randColor() {
  color = {};
  if (Math.round(Math.random())) {
    color['r'] = 255;
  }
  else {
    color['r'] = 0;
  }
  if (Math.round(Math.random())) {
    color['g'] = 255;
  }
  else {
    color['g'] = 0;
  }
  if (Math.round(Math.random())) {
    color['b'] = 255;
  }
  else {
    color['b'] = 0;
  }
  return color;
}

var socket = io.connect('/socket');
socket.on('connect', function() {

});
socket.on('session', function(id) {
  window.session = id;
  if (window.name != "null") {
    $('#cli label').html(window.name + ': ');
  }
  else {
    $('#cli label').html(id + ': ');
  }
});
socket.on('new_value', function(value) {
  $('.progress-bar').css('width', ((value.value / value.max) * 100) + '%');
  $('.progress-bar').html(value.value);
});
socket.on('message', function(data) {
  if (typeof data.client != "undefined") {
    client = data.client;
  }
  else {
    client = 'ChristmasGITree@pi';
  }
  setMessage(data.message, data.status, client);
});

function method(method, value) {
  var data = {};
  var message = '';
  switch(method) {
    case 'on':
      data = {
        type: 'on',
        restore: false
      };
      message = 'Turn on mode';
      break;
    case 'off':
      data = {
        type: 'off',
        restore: false
      };
      message = 'Turn off mode';
      break;
    case 'restore':
      data = {
        type: 'restore'
      };
      message = 'Restore mode';
      break;
    case 'cleanup':
      data = {
        type: 'cleanup'
      };
      message = 'Clean up queue';
      break;
    case 'interrupt':
      data = {
        type: 'interrupt'
      };
      message = 'Interrupt current order';
      break;
    case 'knightRider':
      data = {
        type: 'knightRider'
      };
      message = 'Knight Rider mode';
      break;
    case 'plus':
      data = {
        type: 'plus'
      };
      message = '+1 mode';
      break;
    case 'minus':
      data = {
        type: 'minus'
      };
      message = '-1 mode';
      break;
    case 'disco':
      data = {
        type: 'disco',
        restore: true,
        loops: Math.floor((Math.random() * 10) + 10),
        sleep: Math.random(),
        colors: randColors()
      };
      message = 'Disco mode';
      break;
    case 'name':
      socket.emit('new_name', {id: window.session, name: value});
      $('#cli label').html(value + ': ');
      window.name = value;
      return true;
    case 'message':
      socket.emit('message', {id: window.session, text: value});
      return true;
    default:
      setMessage('Unknown command', 'error', window.session);
      return false;
  }
  sendRequest(data, message);
  return true;
}

$(document).ready(function() {
  $('.method').click(function () {
    method($(this).attr('id'))
  });
  $('#cli input').focus().bind('keypress', function(e) {
    var code = e.keyCode || e.which;
    if(code == 13) {
      var order = $(this).val().split(' ');
      method(order[0], order.slice(1).join(' '));
      $(this).val('').focus();
    }
  });
});
