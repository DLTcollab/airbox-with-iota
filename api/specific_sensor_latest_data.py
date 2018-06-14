from config import HOST_URL
from decode import split
from decode import decode
import requests
import json
from time import gmtime, strftime

def get_specific_sensor_latest_data(uuid):
    payload = {'command':'get_all_claims', 'uuid':uuid}
    response = requests.post(HOST_URL, json=payload)
    claim_list =  response.json()
    sensor_data = []
    epoch = []
    device_id = ''
    for tx in claim_list:
        payload = {'command':'get_claim_info', 'hash_txn':tx}
        response = requests.post(HOST_URL, json=payload)
        msg = split(response.text)[4]
        sensor_data.extend(decode(msg)[0])
        epoch.extend(decode(msg)[1])
        device_id = split(response.text)[4].split('|')[4]
    latest_data_epoch = 0
    latest_data_string = ''
    for i in range(0, len(epoch)):
        if epoch[i] > latest_data_epoch:
            latest_data_epoch = epoch[i]
            latest_data_string = sensor_data[i]
        
    json_string = '{"source":"latest","feeds":[' + latest_data_string + '],"version":"' + strftime("%Y-%m-%dT%H:%M:%SZ", gmtime()) + '","num_of_records":' + '1' +', "device_id": "' + device_id + '"}'
    return json_string
