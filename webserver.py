from flask import Flask, render_template, request
import datetime, json, time, requests, yaml
import array, fcntl, time, signal, sys, getopt
app = Flask(__name__)

spi = file("/dev/spidev0.0", "wb")
fcntl.ioctl(spi, 0x40046b04, array.array('L', [400000]))
rgb = bytearray(3)
step = 0

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
    message = push(ref, master_branch, commits)
    send_to_hipchat(message)
  elif event == "create":
    ref = response['ref']
    message = create(ref)
    send_to_hipchat(message)
  elif event == "pull_request":
    message = pull_request()
    send_to_hipchat(message)
  else:
    return 'This event is not supported', 200

  return 'ok', 200

def push(ref, master_branch, commits):
  # do something
  if ref == master_branch:
    minus(commits)
    return 'boooooo! -' +  str(commits) + ' for you :/'
  else:
    plus(commits)
    return 'Good job! +' + str(commits) + ' for you'

def create(ref):
  # do something
  time.sleep(1)
  return 'test'

def pull_request():
  # do something
  time.sleep(1)
  return 'test'

def plus(number):
  global step
  step = step + number
  set()

def minus(number):
  global step
  step = step - number
  set()

def set():
  global step
  file = open('recipes.json')
  recipes = json.load(file)
  file.close()
  if step > recipes['length'] - 1:
    step = recipes['length'] - 1
    for i in range(0, recipes['length']):
      led(recipes['max'])
  elif step < 0:
    step = 0
    for i in range(0, recipes['length']):
      led(recipes['min'])
  else:
    print 'step:' + str(step)
    for i in range(0, recipes['length']):
      print str(i)
      led(recipes['steps'][step][i])

  spi.flush()

def led(colors):
  rgb[0] = colors['r']
  rgb[1] = colors['g']
  rgb[2] = colors['b']
  spi.write(rgb)

def send_to_hipchat(message):
  stream = open(".settings", 'r')
  settings = yaml.load(stream)
  room_id = settings['room_id']
  auth_token = settings['auth_token']
  host = 'https://api.hipchat.com'
  path = '/v1/rooms/message'

  headers = { 'Content-type': 'application/x-www-form-urlencoded', 'User-Agent': 'RaspberryPi' }
  data = { 'room_id': room_id, 'from': 'ChristmasGITree', 'message': message, 'message_format': 'html', 'color': 'green', 'format': 'json', 'auth_token': auth_token }

  r = requests.post(host + path, data=data, headers=headers)
  # r = requests.post('http://requestb.in/u3x5yau3', data=r.text, headers=headers)
  # r = requests.post('http://requestb.in/u3x5yau3', data=data, headers=headers)

if __name__ == "__main__":
  app.run(host='0.0.0.0', port=80, debug=True)
