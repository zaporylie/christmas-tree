from flask import Flask, render_template, request
import datetime, json, time, requests, yaml
app = Flask(__name__)

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
    message = push(ref)
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

def push(ref):
  # do something
  time.sleep(1)
  return 'test'

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
