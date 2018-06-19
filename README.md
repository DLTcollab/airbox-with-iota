# Airbox-with-IOTA
Integrate [LASS(Location Aware Sensing System)](https://pm25.lass-net.org/) with TangleID and manage sensor data.  
<p align="center">
  <img width="300" src="https://i.imgur.com/bYx1hju.jpg">
  <img width="300" src="https://i.imgur.com/cksjNks.jpg">
</p>

# Getting Started
Follow the [tutorial](https://imgur.com/cksjNks) to build your devices.

## 1. Hardware
* Ameba RTL8195AM
* SHT31 or BME280
* Plantower PMS3003 (G3)
* OLED SSD1306

## 2. Wiring Diagram
### PMS3003 (G3)
Ameba|PMS3003
-|-
5V|pin1
GND|pin2
D1(TX)|pin4
D0(RX)|pin5
<p align="center">
  <img width="1000" src="https://i.imgur.com/tk14fP7.png">
</p>

### SHT31/BME280 and OLED
Ameba|SHT31/BME280|OLED
-|-|-
GND|GND|GND
3.3V|VIN|VCC
SDA|SDA|SDA
SCL|SCL|SCL
3.3V|SD0 (BME280 only)
<p align="center">
  <img width="1000" src="https://i.imgur.com/I7zJcwj.png">
</p>


## 3. Update DAP Firmware (for Windows 10 users)
[How To Update DAP Firmware?](https://www.amebaiot.com/en/change-dap-firmware/)

## 4. Download and install the mbed Windows serial port driver
[Windows serial configuration](https://os.mbed.com/handbook/Windows-serial-configuration#1-download-the-mbed-windows-serial-port)

## 5. Set up Environment
[Ameba Arduino: Getting Started With RTL8195](https://www.amebaiot.com/en/ameba-arduino-getting-started/)

## 6. Update Program To Ameba
### Copy `airbox-with-iota/lib/` to `Documents\Arduino\library`  
<p align="center">
  <img width="500" src="https://i.imgur.com/F4p0Ikw.png">
</p>

### Input your wifi SSID, wifi password, and location in `airbox-with-iota/main/config.h`
<p align="center">
  <img width="500" src="https://i.imgur.com/76TwyR1.png">
</p>

### Update program
<p align="center">
  <img width="500" src="https://i.imgur.com/e0qB5dz.png">
</p>

### Keep your uuid
<p align="center">
  <img width="500" src="https://i.imgur.com/55d7ymf.png">
</p>

### Records
Search `Tag` (`uuid` + `'C'`) on [TheTangle](https://thetangle.org/)  
e.g.  
uuid : LASSPBUAUAXABBMBOBAB  
Tag  : LASSPBUAUAXABBMBOBABC  
https://thetangle.org/tag/LASSPBUAUAXABBMBOBABC

# APIs
* Python 3.6.5
* `python airbox-with-iota/api/test.py`

## `get_uuid_list()`
Search for uuids of all available airboxes.
### Input
none ()
### Return
uuid list (list)

## `get_specific_sensor_all_data()`
Search for all sensor data by an uuid.
### Input
uuid (string)
### Return
sensor data (json)

## `get_specific_sensor_latest_data()`
Search for latest sensor data by an uuid.
### Input
uuid (string)
### Return
sensor data (json)

## `get_specific_sensor_history_data()`
Search for sensor data in one week by an uuid.
### Input
uuid (string)
### Return
sensor data (json)

## `get_specific_sensor_date_data()`
Search for latest sensor data by an uuid and a date.
### Input
uuid (string), yyyy-mm-dd (string)
### Return
sensor data (json)

## `get_all_sensor_latest_data()`
Search for all latest sensor data of each airbox.
### Input
none ()
### Return
sensor data (json)

# References
* [LASS PM2.5 OPEN DATA PORTAL](https://pm25.lass-net.org/)
* [LASS/Airbox and IOTA Integration](https://hackmd.io/s/SJnlK_8yM)
* [(IOTA TangleID) 空氣盒子教學](https://hackmd.io/s/H1HxM-2hz)
* [HACK4U#4: Hack4u 87Live 課程教學](https://paper.dropbox.com/doc/HACK4U4-Hack4u-87Live-TRmqFxXLJHXrKBo4kiR6O)
