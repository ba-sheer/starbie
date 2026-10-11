# starbie
this is my project for week 1 of halflife ysws

<img width="1043" height="478" alt="image" src="https://github.com/user-attachments/assets/a808ab20-aa14-4d60-b7a0-94ae056f4e25" />
<img width="1255" height="643" alt="image" src="https://github.com/user-attachments/assets/29587065-46cd-4978-9c41-bb7a457bb1a0" />


| Component | Quantity |
|---|---:|
| Seeed Studio XIAO ESP32-C3 | 1 |
| 128×64 I²C OLED | 1 |
| DHT11 | 1 |
| MPU6050 | 1 |
| Passive buzzer | 1 |
| WS2812B | 3 |
| MX-style switches | 2 |
| PCB | 1 |


| Function | ESP32-C3 GPIO | XIAO Pin |
|---|---:|---:|
| OLED SDA | GPIO6 | D4 |
| OLED SCL | GPIO7 | D5 |
| DHT11 DATA | GPIO3 | D1 |
| Feed button | GPIO4 | D2 |
| Play button | GPIO5 | D3 |
| WS2812B DIN | GPIO21 | D6 |
| Passive buzzer | GPIO10 | D10 |
| MPU6050 SDA | GPIO6 | D4 |
| MPU6050 SCL | GPIO7 | D5 |

I²C Addresses
```
OLED      0x3C
MPU6050   0x68
```
