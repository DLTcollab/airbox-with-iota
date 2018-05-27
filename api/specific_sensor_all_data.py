from config import HOST_URL
from decode import split
from decode import decode
import requests
import json
from time import gmtime, strftime

def get_specific_sensor_all_data(uuid):
    payload = {'command':'get_all_claims', 'uuid':uuid}
    response = requests.post(HOST_URL, json=payload)
    claim_list =  response.json()
    sensor_data = []
    device_id = ''
    for tx in claim_list:
        payload = {'command':'get_claim_info', 'hash_txn':tx}
        response = requests.post(HOST_URL, json=payload)
        msg = split(response.text)[4]
        sensor_data.extend(decode(msg)[0])
        device_id = split(response.text)[4].split('|')[4]
    sensor_data_string = ''
    for string in sensor_data:
        sensor_data_string += string
        sensor_data_string += ','
    json_string = '{"source":"all","feeds":[' + sensor_data_string[0:len(sensor_data_string)-1] + '],"version":"' + strftime("%Y-%m-%dT%H:%M:%SZ", gmtime()) + '","num_of_records":' + str(len(sensor_data)) +', "device_id": "' + device_id + '"}'
    return json_string
