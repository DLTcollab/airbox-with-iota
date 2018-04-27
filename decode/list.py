import sys
import requests
import json
import time

symbol = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

mapping = {}
mapping["000"] = 1
mapping["001"] = -1
mapping["0100"] = 3
mapping["0101"] = -3
mapping["011"] = 2
mapping["10"] = 0
mapping["110"] = -2
mapping["11100"] = 4
mapping["1110100"] = -5
mapping["111010100"] = -6
mapping["11101010100"] = 7
mapping["1110101010100"] = 8
mapping["1110101010101"] = -8
mapping["1110101010110000000"] = -18
mapping["1110101010110000001"] = 19
mapping["1110101010110000010"] = -21
mapping["1110101010110000011"] = -22
mapping["1110101010110000100"] = 48
mapping["1110101010110000101"] = 41
mapping["1110101010110000110"] = 39
mapping["1110101010110000111"] = 34
mapping["1110101010110001000"] = 25
mapping["1110101010110001001"] = -26
mapping["1110101010110001010"] = 29
mapping["1110101010110001011"] = 46
mapping["1110101010110001100"] = -45
mapping["1110101010110001101"] = 42
mapping["1110101010110001110"] = 49
mapping["1110101010110001111"] = -49
mapping["1110101010110010000"] = -40
mapping["1110101010110010001"] = 33
mapping["1110101010110010010"] = 27
mapping["1110101010110010011"] = -28
mapping["1110101010110010100"] = -20
mapping["1110101010110010101"] = 21
mapping["1110101010110010110"] = -36
mapping["1110101010110010111"] = -39
mapping["1110101010110011000"] = -44
mapping["1110101010110011001"] = -38
mapping["1110101010110011010"] = 22
mapping["1110101010110011011"] = 23
mapping["1110101010110011100"] = 31
mapping["1110101010110011101"] = 43
mapping["1110101010110011110"] = 35
mapping["1110101010110011111"] = -33
mapping["1110101010110100000"] = -24
mapping["1110101010110100001"] = -25
mapping["1110101010110100010"] = -41
mapping["1110101010110100011"] = 37
mapping["1110101010110100100"] = 32
mapping["1110101010110100101"] = -42
mapping["1110101010110100110"] = -23
mapping["1110101010110100111"] = 24
mapping["1110101010110101000"] = 44
mapping["1110101010110101001"] = 40
mapping["1110101010110101010"] = 26
mapping["1110101010110101011"] = -27
mapping["1110101010110101100"] = -37
mapping["1110101010110101101"] = -48
mapping["1110101010110101110"] = 28
mapping["1110101010110101111"] = -32
mapping["1110101010110110000"] = -46
mapping["1110101010110110001"] = 30
mapping["1110101010110110010"] = 16
mapping["1110101010110110011"] = 17
mapping["1110101010110110100"] = -31
mapping["1110101010110110101"] = -30
mapping["1110101010110110110"] = 38
mapping["1110101010110110111"] = -29
mapping["1110101010110111000"] = -47
mapping["1110101010110111001"] = -50
mapping["1110101010110111010"] = 47
mapping["1110101010110111011"] = 45
mapping["1110101010110111100"] = -34
mapping["1110101010110111101"] = -43
mapping["1110101010110111110"] = 36
mapping["1110101010110111111"] = -35
mapping["1110101010111000000"] = -17
mapping["1110101010111000001"] = 18
mapping["111010101011100001"] = 10
mapping["111010101011100010"] = -10
mapping["111010101011100011"] = 11
mapping["111010101011100100"] = -11
mapping["111010101011100101"] = 12
mapping["111010101011100110"] = -12
mapping["111010101011100111"] = -13
mapping["111010101011101"] = -9
mapping["111010101011110"] = 9
mapping["111010101011111000"] = 13
mapping["111010101011111001"] = 14
mapping["111010101011111010"] = -14
mapping["111010101011111011"] = 15
mapping["111010101011111100"] = -15
mapping["111010101011111101"] = -16
mapping["111010101011111110"] = -19
mapping["111010101011111111"] = 20
mapping["1110101011"] = -7
mapping["11101011"] = 6
mapping["111011"] = 5
mapping["1111"] = -4

def decode(msg):
    ver_format, FAKE_GPS, app, ver_app, device_id, ver_tangleID, gps_lon, gps_lat, period, body = msg.split('|')
    period = int(period)

    code = ""
    for x in body:
        for i in range(0,64):
            if x == symbol[i]:
                code = code + "{0:06b}".format(i)
                break
    try:
        #epoch
        epoch = [0 for k in range(10)]
        epoch[0] = int(code[0:32],2)
        code = code[32:len(code)]
        temp = code[0]
        code = code[1:len(code)]
        i = 1
        while i < 10:
            if temp in mapping:
                epoch[i] = epoch[i-1] + mapping[temp] + period
                temp = ""
                i = i + 1
            else:
                temp = temp + code[0]
                code = code[1:len(code)]

        #d0
        d0 = [0 for k in range(10)]
        d0[0] = int(code[0:16],2)
        code = code[16:len(code)]
        temp = code[0]
        code = code[1:len(code)]
        i = 1
        while i < 10:
            if temp in mapping:
                d0[i] = d0[i-1] + mapping[temp]
                temp = ""
                i = i + 1
            else:
                temp = temp + code[0]
                code = code[1:len(code)]

        #d1
        d1 = [0 for k in range(10)]
        d1[0] = int(code[0:16],2)
        code = code[16:len(code)]
        temp = code[0]
        code = code[1:len(code)]
        i = 1
        while i < 10:
            if temp in mapping:
                d1[i] = d1[i-1] + mapping[temp]
                temp = ""
                i = i + 1
            else:
                temp = temp + code[0]
                code = code[1:len(code)]

        #t0
        t0 = [0 for k in range(10)]
        t0[0] = int(code[0:8],2)
        code = code[8:len(code)]
        temp = code[0]
        code = code[1:len(code)]
        i = 1
        while i < 10:
            if temp in mapping:
                t0[i] = t0[i-1] + mapping[temp]
                temp = ""
                i = i + 1
            else:
                temp = temp + code[0]
                code = code[1:len(code)]

        #ho
        h0 = [0 for k in range(10)]
        h0[0] = int(code[0:8],2)
        code = code[8:len(code)]
        temp = code[0]
        code = code[1:len(code)]
        i = 1
        while i < 10:
            if temp in mapping:
                h0[i] = h0[i-1] + mapping[temp]
                temp = ""
                i = i + 1
            else:
                temp = temp + code[0]
                code = code[1:len(code)]
        for j in range(0,10):
            data_str = '{"ver_format": "'+ver_format+'","FAKE_GPS": "'+FAKE_GPS+'","app": "'+app+'","ver_app": "'+ver_app+'","device_id": "'+device_id+'","gps_lon": "'+gps_lon+'","gps_lat": "'+gps_lat+'","timestamp": "'+time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(epoch[j]))+'","s_d0": "'+str(d0[j])+'","s_d1": "'+str(d1[j])+'","s_t0": "'+str(t0[j])+'","s_h0": "'+str(h0[j])+'"},'   
            print(data_str)

    except (IndexError):
        pass

if __name__ == '__main__':
    payload = {'command':'get_all_claims', 'uuid':str(sys.argv[1])}
    response = requests.post("http://node0.puyuma.org/tangleid_backend/api/", json=payload)
    data = response.json()
    index = 0
    for t_hash in data:
        payload = {'command':'get_claim_info', 'hash_txn':t_hash}
        response = requests.post("http://node0.puyuma.org/tangleid_backend/api/", json=payload)
        part_a, part_b, exp_date, claim_pic, msg = response.text.split(',')
        decode(msg) 
    input("Press Enter to continue...")

