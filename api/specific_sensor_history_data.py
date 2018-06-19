from config import HOST_URL
from decode import split
from decode import decode
import requests
import json
import calendar
import time

def get_specific_sensor_history_data(uuid):
    payload = {'command':'get_all_claims', 'uuid':uuid}
    response = requests.post(HOST_URL, json=payload)
    claim_list =  response.json()
    sensor_data = []
    epoch = []
    device_id = ''
    for tx in claim_list:
        try:
            payload = {'command':'get_claim_info', 'hash_txn':tx}
            response = requests.post(HOST_URL, json=payload)
            msg = split(response.text)[5]
            msg = msg[11:len(msg)-1]
            sensor_data.extend(decode(msg)[0])
            epoch.extend(decode(msg)[1])
            device_id = msg.split('|')[4]
        except (IndexError):
            pass
    week_epoch = calendar.timegm(time.gmtime()) - 604800
    history_data = []
    for i in range(0, len(epoch)):
        if epoch[i] > week_epoch:
            history_data.append(sensor_data[i])
    sensor_data_string = ''
    for string in history_data:
        sensor_data_string += string
        sensor_data_string += ','
    json_string = '{"source":"history","feeds":[' + sensor_data_string[0:len(sensor_data_string)-1] + '],"version":"' + time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()) + '","num_of_records":' + str(len(history_data)) +', "device_id": "' + device_id + '"}'
    return json_string
