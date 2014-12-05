from flask import Flask, render_template, request
import datetime, json
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

  ref = response['ref']

  return 'ok', 200

if __name__ == "__main__":
   app.run(host='0.0.0.0', port=80, debug=True)
