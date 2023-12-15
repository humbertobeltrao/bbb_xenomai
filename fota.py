import requests
import time
from requests.adapters import HTTPAdapter
import socket
import subprocess
import random
import string
from ping3 import ping
import threading

#from urllib3.util.retry import Retry

#server_address = 'http://192.168.10.8:80'
server_address = "http://172.22.66.27:80"
#server_address = "http://192.168.43.121:80"

ecu0_ip = "172.22.76.153"
ecu1_ip = "172.22.74.142"
ecu2_ip = "172.22.74.205"
esp32_port = 8080

def is_ip_reachable(ip_address):
    try:
        # Run the ping command with subprocess
        result = subprocess.run(['ping', '-c', '4', ip_address], capture_output=True, text=True, check=True)

        # Store the result in a string
        ping_output = result.stdout

        # Check if 'unreachable' is present in the output
        return 'unreachable' not in ping_output.lower()

    except subprocess.CalledProcessError as e:
        # If the ping command returns an error, print the error message
        print(f"Error: {e.stderr}")
        return False

def wait_until_reachable(ip_address, timeout=30, interval=2):
    start_time = time.time()

    while not is_ip_reachable(ip_address):
        elapsed_time = time.time() - start_time

        if elapsed_time > timeout:
            print(f"Timeout reached ({timeout} seconds). Unable to reach the IP address {ip_address}.")
            return False

        print(f"Waiting for {ip_address} to become reachable. Elapsed time: {elapsed_time:.2f} seconds.")
        time.sleep(interval)

    print(f"The IP address {ip_address} is now reachable.")
    return True

def check_for_update(new_version, new_url, new_ecus):
    try:
        response = requests.post(server_address + '/update_firmware', data={'firmware_version':new_version, 'firmware_url': new_url, 'ecus': new_ecus})
        if(response.status_code == 200):
                updated_firmware_version = response.json()['firmware_version']
                updated_firmware_url = response.json()['firmware_url']
                updated_ecus = response.json()['ecus']
#		print(updated_ecus)
                notify_beaglebone(updated_firmware_version, updated_firmware_url, updated_ecus)
        else:
                print("Failed to update firmware version")
    except Exception as e:
        print("An error occurred: ", str(e))


def id_generator(size=6, chars=string.ascii_uppercase + string.digits):
       return ''.join(random.choice(chars) for _ in range(size))

def notify_beaglebone(updated_firmware_version, updated_firmware_url, updated_ecus):
    print("Received notification from server. Update firmware URL: ", {updated_firmware_version})
    esp32_fw_version = updated_firmware_version+"\n"
    esp32_fw_url = updated_firmware_url+"\n"
    ecu_list = updated_ecus.split(',')
    for ecu in ecu_list:
        try:
                if(ecu == 'ECU 0'):
                        print('ECU 0')
                        esp32_ip = ecu0_ip
                elif(ecu == 'ECU 1'):
                        print('ECU 1')
                        esp32_ip = ecu1_ip
                else:
                        print('ECU 2')
                        esp32_ip = ecu2_ip
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect((esp32_ip, esp32_port))
                client_socket.sendall(esp32_fw_version.encode("utf-8"))
                client_socket.sendall(esp32_fw_url.encode("utf-8"))
                client_socket.close()
                time.sleep(30)
                while(True):
                    try:
                        result = ping(esp32_ip, timeout=1)
                        if result is not None: 
                            print("Process finished for the current ECU.")
                            response = requests.post(server_address + '/update_status', data={'device':ecu, 'status':"DONE", 'size': len(ecu_list), 'code': id_generator()})
                            if response.status_code == 200:
                                print("Status update sent successfully to the server.") 
                                break
                    except Exception as e:
                        print("Error pinging URL {e}")
        except Exception as e:
            print('Error sending URL', str(e))
'''
def check_health_status(ecus):
    while True:
        try:
            for ecu in ecus:
                if ecu == 'ECU 0':
                    esp32_ip = ecu0_ip
                elif ecu == 'ECU 1':
                    esp32_ip = ecu1_ip
                else:
                    esp32_ip = ecu2_ip
                response = requests.get(esp32_ip + '/status')
                if response.status_code == 200:
                    try:
                        resp_post = requests.post(server_address + '/get_health_status')
                        if resp_post.status_code == 200:
                            print("Health status sent successfully to the server.")
                            break
                    except Exception as e:
                        print("Error sending health status {e}")
                    finally:
                        time.sleep(5)
        except Exception as e:
            print('Error sending URL for health status', str(e))
'''

def list_for_updates():
        current_version = ""
        while True:
                try:
                    response = requests.get(server_address + '/get_firmware_update')
                    if response.status_code == 200:
                        previous_version = current_version
                        current_version = response.json()['firmware_version']
                        current_url = response.json()['firmware_url']
                        ecus = response.json()['ecus']
                        #health_check_thread = threading.Thread(target=check_health_status(ecus))
                        #health_check_thread.start()
			#print (ecus)
                        if previous_version != current_version:
                            check_for_update(current_version, current_url, ecus)
                except Exception as e:
                     print("An error occurred: ", str(e))
                finally:
                     time.sleep(1)

if __name__ == "__main__":
    list_for_updates()
