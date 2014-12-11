$( document ).ready(function() {

  $('.call.on').click(function () {
    data = {
      'type': 'on',
      'restore': false
    }
    callAjaxPlay(data, 'Turned on!');
  });

  $('.call.off').click(function () {
    data = {
      'type': 'off',
      'restore': false
    }
    callAjaxPlay(data, 'Turned off!');
  });

  $('.call.restore').click(function () {
    data = {
      'type': 'restore'
    }
    callAjaxPlay(data, 'Restored!');
  });

  $('.call.knightrider').click(function () {
    data = {
      'type': 'knightRider'
    }
    callAjaxPlay(data, 'Knight Rider!');
  });

  $('.call.disco').click(function () {
    data = {
      'type': 'disco',
      'restore': true,
      'loops': Math.floor((Math.random() * 10) + 10),
      'sleep': Math.random(),
      'colors': randColors()
    }
    callAjaxPlay(data, 'Disco!');
  });


  function callAjaxPlay(data, message) {
    $.ajax({
      url: "/play",
      method: 'POST',
      contentType: "application/json",
      dataType: 'json',
      headers: {
        'Awesome-Security': 'Value'
      },
      data: JSON.stringify(data)
    }).done(function(data) {
      $("#console").prepend("<div class='bs-callout bs-callout-success'>" + message + "</div>");
    }).fail(function(data) {
      $("#console").prepend("<div class='bs-callout bs-callout-danger'>Failed: " + message + "</div>");
    });
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
});