from decode import split
from decode import decode
from uuid_list import get_uuid_list
from specific_sensor_all_data import get_specific_sensor_all_data
from specific_sensor_latest_data import get_specific_sensor_latest_data
from specific_sensor_history_data import get_specific_sensor_history_data
from specific_sensor_date_data import get_specific_sensor_date_data
from all_sensor_latest_data import get_all_sensor_latest_data

print('\n\n\n*****  Test : split()  *****\n')
print('Input (type):\tget_claim_info return value (string)')
print('Output(type):\t[a_part (string), b_part (string), exp_date (string), pic (string), msg (string)]\n')
return_value = 'LASSPBUAUAXABBMBOBAB,LASSPBUAUAXABBMBOBAB,,,3|1|PM25|live|8CE7A927|0|120.971042|24.801909|60|WwJioKqdQAJSHToAAmeHTLJDVVUeA0ao'
print(split(return_value))

print('\n\n\n*****  Test : decode()  *****\n')
print('Input (type):\tmsg (string)')
print('Output(type):\t[sensor_data (string), timestamp (string)]\n')
msg = '3|1|PM25|live|8CE7A927|0|120.971042|24.801909|60|WwJioKqdQAJSHToAAmeHTLJDVVUeA0ao'
print(decode(msg)) 

print('\n\n\n*****  Test : get_uuid_list()  *****\n')
print('Input (type):\tnone ()')
print('Output (type):\tuuid list (list)\n')
print(get_uuid_list())

print('\n\n\n*****  Test : get_specific_sensor_all_data()  *****\n')
print('Input (type):\tuuid (string)')
print('Output (type):\tsensor data (json)\n')
uuid = 'LASSPBUAUAXABBMBOBAB'
print(get_specific_sensor_all_data(uuid))

print('\n\n\n*****  Test : get_specific_sensor_latest_data()  *****\n')
print('Input (type):\tuuid (string)')
print('Output (type):\tsensor data (json)\n')
uuid = 'LASSPBUAUAXABBMBOBAB'
print(get_specific_sensor_latest_data(uuid))

print('\n\n\n*****  Test : get_specific_sensor_history_data()  *****\n')
print('Input (type):\tuuid (string)')
print('Output (type):\tsensor data (json)\n')
uuid = 'LASSPBUAUAXABBMBOBAB'
print(get_specific_sensor_history_data(uuid))

print('\n\n\n*****  Test : get_specific_sensor_date_data()  *****\n')
print('Input (type):\tuuid (string), date y-m-d (string)')
print('Output (type):\tsensor data (json)\n')
uuid = 'LASSPBUAUAXABBMBOBAB'
date = '2018-06-18'
print(get_specific_sensor_date_data(uuid, date))

print('\n\n\n*****  Test : get_all_sensor_latest_data()  *****\n')
print('Input (type):\tnone ()')
print('Output (type):\tsensor data (json)\n')
print(get_all_sensor_latest_data())
