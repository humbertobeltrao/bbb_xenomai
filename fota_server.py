from flask import Flask, render_template, jsonify, request, session



app = Flask(__name__)

# Variable to store the current LED frequency
current_url = "https://raw.githubusercontent.com/humbertobeltrao/bbb_xenomai/main/bins/fw_three.bin"
current_version = "https://raw.githubusercontent.com/humbertobeltrao/bbb_xenomai/main/versions/bin-version1.txt"
current_ecus = []

progress_data = {}

device = ""
new_status = ""
size = ""

health_status = {}

# Route for the web interface
@app.route('/')
def index():
    return render_template('layout.html')

# Route for sending the LED frequency update command
@app.route('/update_firmware', methods=['POST'])
def update_firmware():
    new_firmware_url = request.form['firmware_url']
    new_firmware_version = request.form['firmware_version']
    new_ecus = request.form['ecus']
    global current_url;
    global current_version;
    global current_ecus;
    current_url = new_firmware_url
    current_version = new_firmware_version
    current_ecus = new_ecus
    print(current_ecus)
    return jsonify({'firmware_version' : current_version, 'firmware_url': current_url, 'ecus' : current_ecus})

@app.route('/get_firmware_update', methods=['GET'])
def get_firmware_update():
    global current_url
    global current_version
    global current_ecus
    return jsonify({'firmware_version' : current_version, 'firmware_url': current_url, 'ecus': current_ecus})


@app.route('/update_status', methods=['POST'])
def update_status():
    try:
        data = request.form

        global device
        global new_status
        global size
        global code 
        device = data['device']
        new_status = data['status']
        size = data['size']
        code = data['code']
        print(device + new_status)

        return jsonify({'device': device, 'status': new_status, 'size': size, 'code': code})
    except Exception as e:
        return jsonify({'success': False, 'message': str(e)})

@app.route('/get_status_update', methods=['GET'])
def get_status_update():
    global device
    global new_status
    global size
    global code 
    return jsonify({'device': device, 'status': new_status, 'size':size, 'code': code})


def update_health_status(ecu, status):
    health_status[ecu] = status

# Route for getting health status
@app.route('/get_health_status', methods=['GET'])
def get_health_status():
    global health_status
    return jsonify(health_status)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=80)
