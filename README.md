 # You Safe Bolt

3rd - Year Project Course (SYSC3010)

Traditional products are increasingly being replaced with various, sometimes complicated, consumer grade electronics. The You Safe Bolt provides the benefit of having everything needed to secure a home in one place, centralized on the in-home server and accessible at any point through the mobile phone app. There is no need for keys, remembering passcodes, or proximity access key fobs. However, the system can be customized to use a passcode at any point.  

There are three ways to access the lock:  
 1) Fingerprint Scanner 
 2) Mobile App (You Safe Bolt) 
 3) Passcode 


![alt text](https://github.com/martinklamrowski/You_Safe_Bolt/blob/master/misc/readme.jpg)


## Components

* Raspberry Pi 1, Raspberry Pi 3B, Raspberry Pi 2
* Arduino Uno
* Adafruit ZFM-20 Fingerprint Sensor
* 4x4 Keypad
* 16x2 LCD
* Stepper Motor


## Setup Notes

**Server RPi Setup**

The server RPi was configured to use a static IP over a wired interface. To do this, add these lines to the following files (you can use nano or vim from a terminal). 

/etc/network/interfaces
```
iface eth0 inet manual
```

/etc/dhcpcd.conf
```
interface eth0
static ip_address=192.168.0.21/24
static routers=192.168.0.1
static domain_name_servers=192.168.0.1
```

**Client RPi Setup**

The client RPi can be configured to use a static IP as well however it is not necessary. The interface is wireless in this case. 

/etc/network/interfaces
```
iface wlan0 inet manual
```

/etc/dhcpcd.conf
```
interface wlan0
static ip_address=192.168.0.220/24
static routers=192.168.0.1
static domain_name_servers=192.168.0.1
```

The final step necessary is to ensure that the camera is enabled. You can check by typing -sudo raspi-config in a terminal, and navigating to "Interfacing Options".

### Deployment

This system is intended to be used on a local area network. A wireless router is recommended. 

To deploy this system: 
* download the lock directory onto the client RPi, and the server directory onto the server RPi
* connect all hardware
* run main.py for each RPi 

## Authors

* **Martin Klamrowski** - *Client, Server* - [github](https://github.com/martinklamrowski)
* **Tufayl Dhalla** - *Hardware, Arduino* - [github](https://github.com/tufayldhalla)
* **Birat Dhungana** - *Mobile App* - [github](https://github.com/biratdhungana)
* **Angie Byun** - *Client, Server, Testing* - [github](https://github.com/angiebyun)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* Ksheel for the 3D print
