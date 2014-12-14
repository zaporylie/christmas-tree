from flask import Flask, render_template, request, jsonify
import datetime, json, time, requests, yaml
import array, fcntl, time, signal, sys, random, re
from flask.ext.socketio import SocketIO, emit
import gevent
from gevent.queue import Queue

app = Flask(__name__, static_url_path='/static')
app.config['SECRET_KEY'] = 'secret!'
app.config['DEBUG'] = True

socketio = SocketIO(app)

q = Queue()

spi = file("/dev/spidev0.0", "wb")
fcntl.ioctl(spi, 0x40046b04, array.array('L', [400000]))
interrupt = False

# Message Class
class Message:
  def getSettings(self):
    stream = open(".settings.strings", 'r')
    x = yaml.load(stream)
    stream = open(".settings", 'r')
    y = yaml.load(stream)
    self.settings = dict(x.items() + y.items())

  def __init__(self, direction, sender, points):
    self.getSettings()
    self.message = '<table style="width: 100%"><tr><td>' + self.senderAvatar(sender) + '</td><td>' + self.getRandomText(direction) % {'author': self.senderName(sender), 'number': points} + '</td></tr></table>'
    self.send()

  def getRandomText(self, direction):
    if direction == 'plus':
      self.color = 'green'
      return random.choice(self.settings[direction])
    elif direction == 'minus':
      self.color = 'red'
      return random.choice(self.settings[direction])
    else:
      return 'Unknown text'

  def senderName(self, sender):
    return '<a href="' + sender['html_url'] + '">' + sender['login'] + '</a>'

  def senderAvatar(self, sender):
    return '<img src="' + sender['avatar_url'] + '" width="32" height="32" hspace="5" vspace="5" />'

  def send(self):
    host = 'https://api.hipchat.com'
    path = '/v1/rooms/message'
    headers = { 'Content-type': 'application/x-www-form-urlencoded', 'User-Agent': 'RaspberryPi' }
    data = { 'room_id': self.settings['room_id'], 'from': self.settings['author'], 'message': self.message, 'message_format': 'html', 'color': self.color, 'format': 'json', 'auth_token': self.settings['hipchat_secret'] }
    r = requests.post(host + path, data=data, headers=headers)

# Christmas Tree class
class ChristmasTree():
  value = 0;
  settings = [];

  def __init__(self):
    self.getSettings()
    self.value = self.settings['initial_value']

  def run(self):
    while True:
      try:
        task = q.get()
        func = task['command']
        args = task['parameters']
        f = getattr(self, func)

        response = {
          'status': 'ok',
          'message': 'Starting: {}' . format(task['command']),
        }
        try:
          response['client'] = task['client']
        except:
          response['client'] = self.settings['author']

        # send message
        socketio.emit('message', response, namespace='/socket')

        if f(args) == True:
          response = {
            'status': 'ok',
            'message': '{} completed!' . format(task['command']),
          }
          try:
            response['client'] = task['client']
          except:
            response['client'] = self.settings['author']
          # send message
          socketio.emit('message', response, namespace='/socket')
        else:
          response = {
            'status': 'error',
            'message': '{} failed!' . format(task['command']),
          }
          try:
            response['client'] = task['client']
          except:
            response['client'] = self.settings['author']

          # send message
          socketio.emit('message', response, namespace='/socket')

      except Exception, e:#little bit ugly
        print e

  def getSettings(self):
    stream = open(".settings", 'r')
    self.settings = yaml.load(stream)

  def getValue(self):
    return self.value

  def plus(self, number):
    self.value += number
    if self.value >= 50:
      self.value = 49
    self.set()
    socketio.emit('new_value', { 'value': self.value + 1, 'max': self.settings['num_leds'] }, namespace='/socket')
    return True

  def minus(self, number):
    self.value -= number
    if self.value < 0:
      self.value = 0
    self.set()
    socketio.emit('new_value', { 'value': self.value + 1, 'max': self.settings['num_leds'] }, namespace='/socket')
    return True

  def set(self):
    # Rebuild settings
    self.getSettings()
    # Get random colored lights
    rand = []
    rand.append(random.randint(0,8))
    rand.append(random.randint(9,16))
    rand.append(random.randint(17,24))
    rand.append(random.randint(25,32))
    rand.append(random.randint(33,40))
    rand.append(random.randint(41,48))

    for i in range(0, self.settings['num_leds']):
      if i < self.value and i in rand:
        self.writeLed(
          {
            'r': random.randint(0,255),
            'g': 0,
            'b': 0
          }
        )
      elif i == (self.settings['num_leds'] - 1) and self.value == (self.settings['num_leds'] - 1):
        self.writeLed({'r': 255, 'g': 150, 'b': 0})
      elif i < self.value:
        self.writeLed({'r': 0, 'g': 200, 'b': 0})
      else:
        self.writeLed({'r': 0, 'g': 0, 'b': 0})

    spi.flush()
    return True

  def writeLed(self, color):
    rgb = bytearray(3)
    rgb[0] = color['r']
    rgb[1] = color['g']
    rgb[2] = color['b']
    spi.write(rgb)
    return True

  def blinkMode(self, json):
    global interrupt

    # Rebuild settings
    self.getSettings()

    try:
      json['loops']
    except:
      print('No loops in your request')
      return False
    
    for i in range(0, json['loops']):
      for j in range(0, self.settings['num_leds']):

        # kill process on interrupt
        if interrupt == True:
          interrupt = False
          spi.flush()
          return False

        try:
          # test if value exist
          self.writeLed(json['values'][j])
        except:
          self.writeLed({'r': 0, 'g': 0, 'b': 0})

      spi.flush()

      try:
        time.sleep(json['sleep'])
      except:
        print('No sleep value, default - 0.2')
        time.sleep(0.2)

      
      # Zero out the leds.
      for i in range(0, self.settings['num_leds']):

        # kill process on interrupt
        if interrupt == True:
          interrupt = False
          spi.flush()
          return False

        self.writeLed({'r': 0, 'g': 0, 'b': 0})

      spi.flush()

      try:
        time.sleep(json['sleep'])
      except:
        time.sleep(0.2)

    return True

  def on(self, json):
    tmp = self.value
    self.value = self.settings['num_leds'] - 1
    self.set()
    self.value = tmp
    return True
    
  def off(self, json):
    tmp = self.value
    self.value = 0
    self.set()
    self.value = tmp
    return True

  def restore(self, json):
    self.set()
    return True

  def singleLoop(self, start, end, t, frame):
    global interrupt

    for i in range(start, end, t):

      # kill process on interrupt
      if interrupt == True:
        interrupt = False
        spi.flush()
        return False

      k = 0
      for j in range(0, i):
        self.writeLed({'r': 0, 'g': 0, 'b': 0})
        k += 1
      for j in frame:
        self.writeLed(j)
        k += 1
      for j in range(k, self.settings['num_leds']):
        self.writeLed({'r': 0, 'g': 0, 'b': 0})

      spi.flush()

      time.sleep(0.03)

    return True

  def knightRider(self, json):
    try:
      frame = json['frame']
    except:
      frame = [
        {
          'r': 20,
          'g': 0,
          'b': 0,
        },
        {
          'r': 60,
          'g': 0,
          'b': 0,
        },
        {
          'r': 90,
          'g': 0,
          'b': 0,
        },
        {
          'r': 150,
          'g': 0,
          'b': 0,
        },
        {
          'r': 90,
          'g': 0,
          'b': 0,
        },
        {
          'r': 60,
          'g': 0,
          'b': 0,
        },
        {
          'r': 20,
          'g': 0,
          'b': 0,
        },
      ]

    for i in range(0, 5):
      if self.singleLoop(0, self.settings['num_leds'] - len(frame), 1, frame) == False:
        print False
        return False
      if self.singleLoop(self.settings['num_leds'] - len(frame), 0, -1, frame) == False:
        print False
        return False

    return True

  def disco(self, json):
    global interrupt

    try:
      colors = json['colors']
    except:
      print('Missing colors element')
      return False

    for i in range(0, json['loops']):
      for i in range(0, self.settings['num_leds']):

        # kill process on interrupt
        if interrupt == True:
          interrupt = False
          spi.flush()
          return False

        color = json['colors'][random.randint(0, len(colors) - 1)]
        self.writeLed(color)
  
      spi.flush()

      try:
        time.sleep(json['sleep'])
      except:
        time.sleep(0.5)

    return True


# define queue
q = Queue()


def push(response):
  branch = response['ref']
  if branch.startswith('refs/heads/'):
    branch = re.sub('refs/heads/', '', branch);
  master_branch = response['repository']['master_branch']
  sender = response['sender']
  points = len(response['commits'])
  is_merged = response['head_commit']['message'].startswith('Merge pull request')

  # do something
  if branch == master_branch and not is_merged:
    q.put({'command': 'minus', 'parameters': points, 'client': sender['login']})
    Message('minus', sender, points)
  elif branch == master_branch and is_merged:
    q.put({'command': 'plus', 'parameters': 3, 'client': sender['login']})
    Message('plus', sender, 3)
  else:
    q.put({'command': 'plus', 'parameters': points, 'client': sender['login']})
    Message('plus', sender, points)

def create(sender):
  q.put({'command': 'plus', 'parameters': 2, 'client': sender['login']})
  Message('plus', sender, 2)

def pull_request(sender):
  q.put({'command': 'plus', 'parameters': 2, 'client': sender['login']})
  Message('plus', sender, 2)

def issue_comment(sender):
  q.put({'command': 'plus', 'parameters': 1, 'client': sender['login']})
  Message('plus', sender, 1)


@app.route("/", methods=['GET'])
def index():
  return render_template('index.html')

@app.route("/endpoint", methods=['GET', 'POST'])
def endpoint():

  # Check if header is correct
  if request.headers.get('X-GitHub-Event') is None:
    return "X-Github-Event header required", 400;

  event = request.headers.get('X-GitHub-Event')

  # Get JSON from input.
  response = request.get_json()

  if event == "push":

    push(response)

  elif event == "create":

    create(response['sender'])

  elif event == "pull_request":

    pull_request(response['sender'])

  elif event == "issue_comment":

    issue_comment(response['sender'])

  else:

    return 'This event is not yet supported', 200

  return 'ok', 200

@app.route("/interface", methods=['GET'])
def interface():

  # Prepare data
  templateData = {
    'value': GITree.getValue() + 1,
    'max': GITree.settings['num_leds'],
    'percentage': (float(GITree.getValue() + 1) / float(GITree.settings['num_leds'] )) * 100
  }

  # Render interface
  return render_template('interface.html', **templateData)

@socketio.on('connect', namespace='/socket')
def connect():
  id = random.getrandbits(128)
  socketio.emit('session', id, namespace='/socket')
  socketio.emit('message', {'status': 'ok', 'message': 'User {} is online' . format(id)}, namespace='/socket')

@socketio.on('message', namespace='/socket')
def message(message):
  socketio.emit('message', {'status': 'default', 'message':  message['text'], 'client': message['id']}, namespace='/socket')

@socketio.on('disconnect', namespace='/socket')
def disconnect():
  socketio.emit('message', {'status': 'error', 'message': 'User disconnected'}, namespace='/socket')

@socketio.on('method', namespace='/socket')
def method(json):
  print(json)
  availableTypes = [
    'blink',
    'on',
    'off',
    'restore',
    'knightRider',
    'disco',
  ]

  if json['type'] == 'interrupt':

    # Set global interrupt flag
    global interrupt
    interrupt = True

    # Set response
    message = 'Current order has been interrupted'

  elif json['type'] == 'cleanup':

    # Manually clean queue up
    while not q.empty():
      task = q.get()
      gevent.sleep(0)

    # Set response
    message = 'Orders queue has been cleaned'

  elif json['type'] == 'plus' or json['type'] == 'minus':
    q.put({'command': json['type'], 'parameters': 1})

    # Set message
    message = json['type'] + ' has been queued'

  elif json['type'] in availableTypes:

    # Add task to queue
    q.put({'command': json['type'], 'parameters': json})

    # Set message
    message = json['type'] + ' has been queued'

  else:

    # Set error message
    error = 'This event is not supported yet'

  try:
    response = {
      'status': 'error',
      'message': error
    }
  except:
    response = {
      'status': 'ok',
      'message': message
    }

  # Send message to all clients
  socketio.emit('message', response, namespace='/socket')


if __name__ == "__main__":

  # define new Christmas tree object
  GITree = ChristmasTree()

  thread = gevent.spawn(GITree.run)

  # app.run(host='0.0.0.0', port=80, debug=True)
  socketio.run(app)
