from flask import Flask, render_template, request
import datetime, json
app = Flask(__name__)

@app.route("/", methods=['GET', 'POST'])
def hello():
   # now = datetime.datetime.now()
   # timeString = now.strftime("%Y-%m-%d %H:%M")
   response = request.get_json()
   string = response['ref']
   templateData = {
      'title' : 'HELLO!',
      'ref': string
      }
   return render_template('main.html', **templateData)

if __name__ == "__main__":
   app.run(host='0.0.0.0', port=80, debug=True)
