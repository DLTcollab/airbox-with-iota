from config import HOST_URL
from decode import split
from decode import decode
from uuid_list import get_uuid_list
import requests
import json
import time

def get_all_sensor_latest_data():
    uuid_list = get_uuid_list()
    all_latest_data = []
    for uuid in uuid_list:        
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
        latest_data_epoch = 0
        latest_data_string = ''
        for i in range(0, len(epoch)):
            if epoch[i] > latest_data_epoch:
                latest_data_epoch = epoch[i]
                latest_data_string = sensor_data[i]
        if latest_data_string:
            all_latest_data.append(latest_data_string)
    all_latest_data_string = ''
    for string in all_latest_data:
        all_latest_data_string += string
        all_latest_data_string += ','
    json_string = '{"source":"all latest","feeds":[' + all_latest_data_string[0:len(all_latest_data_string)-1] + '],"version":"' + time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()) + '","num_of_records":' + str(len(all_latest_data)) + '}'
    return json_string
