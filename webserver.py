from flask import Flask, render_template, request, jsonify
import datetime, json, time, requests, yaml
import array, fcntl, time, signal, sys, random, re, threading
from Queue import Queue
app = Flask(__name__, static_url_path='/static')

spi = file("/dev/spidev0.0", "wb")
fcntl.ioctl(spi, 0x40046b04, array.array('L', [400000]))

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
class ChristmasTree(threading.Thread):
  value = 0;
  settings = [];

  def __init__(self, queue):
    self.getSettings()
    self.value = self.settings['initial_value']
    self.queue = queue
    threading.Thread.__init__(self)

  def run(self):
    while True:
      order = self.queue.get()
      self.queue.task_done()
      try:
        func = order[0]
        args = order[1:]
        self.func(*args)

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

  def minus(self, number):
    self.value -= number
    if self.value < 0:
      self.value = 0
    self.set()

  def set(self):
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
            'g': random.randint(0,10),
            'b': random.randint(0,10)
          }
        )
      elif i == 49 and self.value == 49:
        self.writeLed({'r': 255, 'g': 150, 'b': 0})
      elif i < self.value:
        self.writeLed({'r': 0, 'g': 200, 'b': 0})
      else:
        self.writeLed({'r': 0, 'g': 0, 'b': 0})

    spi.flush()

  def writeLed(self, color):
    rgb = bytearray(3)
    rgb[0] = color['r']
    rgb[1] = color['g']
    rgb[2] = color['b']
    spi.write(rgb)

  def blinkMode(self, json):
    self.getSettings()
    try:
      json['loops']
    except:
      print('No loops in your request')
      return false
    
    for i in range(0, json['loops']):
      for j in range(0, self.settings['num_leds']):
        try:
          # test
          self.writeLed(json['values'][j])
        except:
          self.writeLed({'r': 0, 'g': 0, 'b': 0})

      spi.flush()
      try:
        time.sleep(json['sleep'])
      except:
        print('No sleep value')
      
      # Zero out the leds.
      for i in range(0, self.settings['num_leds']):
        self.writeLed({'r': 0, 'g': 0, 'b': 0})
      spi.flush()
      try:
        time.sleep(json['sleep'])
      except:
        print('No sleep value')
        
  def on(self, json):
    tmp = self.value
    self.value = 49
    self.set()
    self.value = tmp
    
  def off(self, json):
    tmp = self.value
    self.value = 0
    self.set()
    self.value = tmp

  def singleLoop(self, start, end, t, frame):
    for i in range(start, end, t):
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

  def knightRider(self, json):
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
      self.singleLoop(0, self.settings['num_leds'] - len(frame), 1, frame)
      self.singleLoop(self.settings['num_leds'] - len(frame), 0, -1, frame)


  def disco(self, json):
    colors = json['colors']
    for i in range(0, json['loops']):
      for i in range(0, self.settings['num_leds']):
        color = json['colors'][random.randint(0, len(json['colors']) - 1)]
        self.writeLed(color)
  
      spi.flush()
      try:
        time.sleep(json['sleep'])
      except:
        time.sleep(0.5)

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
    # GITree.minus(points)
    Message('minus', sender, points)
  elif branch == master_branch and is_merged:
    # GITree.plus(3)
    Message('plus', sender, 3)
  else:
    # GITree.plus(points)
    Message('plus', sender, points)

def create(sender):
  # GITree.plus(2)
  Message('plus', sender, 2)

def pull_request(sender):
  # GITree.plus(2)
  Message('plus', sender, 2)

def issue_comment(sender):
  # GITree.plus(1)
  Message('plus', sender, 1)

# define queue
q = Queue()

@app.route("/", methods=['GET'])
def index():
  return render_template('index.html')

@app.route("/endpoint", methods=['GET', 'POST'])
def endpoint():
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

@app.route("/play", methods=['GET', 'POST'])
def play():
  if request.headers.get('Awesome-Security') is None:
    return "Awesome-Security is enabled", 400;

  # Get JSON from input.
  json = request.get_json()

  if json['type'] == "blink":
    # GITree.blinkMode(json)
    q.put(blinkMode, json)
  elif json['type'] == "on":
    # GITree.on(json)
    q.put(on, json)
  elif json['type'] == "off":
    # GITree.off(json)
    q.put(off, json)
  elif json['type'] == "restore":
    # GITree.set()
    q.put(restore, json)
  elif json['type'] == "knightRider":
    # GITree.knightRider(json)
    q.put(knightRider, json)
  elif json['type'] == 'disco':
    # GITree.disco(json)
    q.put(disco, json)
  else:
    error = 'This event is not supported yet'

  try:
    response = {
      'status': 'error',
      'message': error
    }
  except:
    response = {
      'status': 'ok',
    }

  return jsonify(**response), 200

@app.route("/interface", methods=['GET'])
def interface():
  templateData = {
    'value': GITree.getValue() + 1,
    'max': GITree.settings['num_leds'],
    'percentage': (float(GITree.getValue() + 1) / float(GITree.settings['num_leds'] )) * 100
  }
  return render_template('interface.html', **templateData)

if __name__ == "__main__":
  # define new Christmas tree object
  GITree = ChristmasTree(q)
  # start a thread
  GITree.start()
  app.run(host='0.0.0.0', port=80, debug=True)
