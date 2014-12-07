from flask import Flask, render_template, request
import datetime, json, time, requests, yaml
import array, fcntl, time, signal, sys, random
app = Flask(__name__)

spi = file("/dev/spidev0.0", "wb")
fcntl.ioctl(spi, 0x40046b04, array.array('L', [400000]))

def push(ref, master_branch, commits):
  # do something
  if ref == master_branch:
    GITree.minus(commits)
  else:
    GITree.plus(commits)

def create(ref):
  # do something
  time.sleep(1)
  return 'test'

def pull_request():
  # do something
  time.sleep(1)
  return 'test'



def send_to_hipchat(message):
  stream = open(".settings", 'r')
  settings = yaml.load(stream)

  host = 'https://api.hipchat.com'
  path = '/v1/rooms/message'

  headers = { 'Content-type': 'application/x-www-form-urlencoded', 'User-Agent': 'RaspberryPi' }
  data = { 'room_id': settings['room_id'], 'from': settings['author'], 'message': message, 'message_format': 'html', 'color': 'green', 'format': 'json', 'auth_token': settings['hipchat_secret'] }

  r = requests.post(host + path, data=data, headers=headers)
  # r = requests.post('http://requestb.in/u3x5yau3', data=r.text, headers=headers)
  # r = requests.post('http://requestb.in/u3x5yau3', data=data, headers=headers)

class ChristmasTree:
  value = 0;
  settings = [];

  def __init__(self):
    self.value = 0;
    stream = open(".settings", 'r')
    self.settings = yaml.load(stream)

  def getValue(self):
    return self.value

  def plus(self, number):
    self.value += number
    if self.value >= len(self.settings['steps']):
      self.value = len(self.settings['steps']) - 1
    self.set()

  def minus(self, number):
    self.value -= number
    if self.value < 0:
      self.value = 0
    self.set()

  def getRandomText(self, direction):
    if direction == 'plus' or direction == 'minus':
      return random.choice(self.settings[direction])
    else:
      return 'Unknown text'

  def set(self):
    for i in range(0, self.settings['num_leds']):
      try:
        self.writeLed(self.settings['steps'][self.value][i])
      except:
        self.writeLed({'r': 0, 'g': 0, 'b': 0})

    spi.flush()

  def writeLed(self, color):
    rgb = bytearray(3)
    rgb[0] = color['r']
    rgb[1] = color['g']
    rgb[2] = color['b']
    spi.write(rgb)

# define new Christmas tree object
GITree = ChristmasTree()

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
    ref = response['ref']
    master_branch = response['repository']['master_branch']
    commits = len(response['commits'])
    push(ref, master_branch, commits)
  elif event == "create":
    ref = response['ref']
    create(ref)
  elif event == "pull_request":
    pull_request()
  else:
    return 'This event is not supported', 200

  return 'ok', 200

if __name__ == "__main__":
  app.run(host='0.0.0.0', port=80, debug=True)
