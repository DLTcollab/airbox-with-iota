from config import HOST_URL
from decode import split
import requests

def get_uuid_list():
    payload = {'command':'get_all_claims', 'uuid':'LASSUUIDLIST'}
    response = requests.post(HOST_URL, json=payload)
    claim_list =  response.json()
    uuid_list = []
    for tx in claim_list:
        try:
            payload = {'command':'get_claim_info', 'hash_txn':tx}
            response = requests.post(HOST_URL, json=payload)
            uuid = split(response.text)[5]
            uuid = uuid[11:len(uuid)-1]
            if uuid not in uuid_list:
                uuid_list.append(uuid)
        except (IndexError):
            pass
    return uuid_list
