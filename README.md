# starbie
this is my project for week 1 of halflife ysws.my goal is to build a keychain tamagotchi based on guide provided by hc

this is the link of half life project page https://halflife.hackclub.com/projects/cmuyamfi1004z01qvesjx09nr

The idea is simple: take care of a tiny digital pet by feeding it, playing with it, and shaking the device to interact with it. An OLED screen displays the pet, while RGB LEDs show its current status.

I'm using a Seeed Studio XIAO ESP32-C3 as the main controller, along with an OLED display, an MPU6050 for motion detection, buttons, and a few other components.

this project is basicly a tamagothchi i am planning to use as my backpacks key chain and It is week 1 of learning the hardware series

fully funded by hackclub

# bom

| Item                     | Qty | Supplier      | Cost/item | Total Cost | link                                                                                                                       |
|--------------------------|-----|---------------|-----------|------------|----------------------------------------------------------------------------------------------------------------------------|
| XIAO ESP32-C3            | 1   | techism       | $6.00     | $6.00      | https://techiesms.com/product/seeed-studio-xiao-esp32c3/?srsltid=AU7gw4X0ypi5G6QPsjTj6_fCxW_KTHcVgSjSWs9jFoDQ4cQplrFg8Iaqx |
| Mechanical switches      | 1   | cosmic byte   | $4.12     | $4.12      | https://www.thecosmicbyte.com/product/outemu-mechanical-switches-for-swappable-keyboards-pack-of-20/                       |
| OLED display             | 1   | techism       | $2.00     | $2.00      | https://techiesms.com/product/0-96-inch-i2c-iic-4pin-128x64-oled-disp/                                                     |
| MPU6050 module           | 1   | techism       | $2.00     | $2.00      | https://techiesms.com/product/mpu6050-3-axis-accelerometer-and-gyro-sensor/                                                |
| dht11                    | 1   | robocraze     | $0.70     | $0.70      | https://robocraze.com/products/dht11-humidity-and-temperature-sensor                                                       |
| passive buzzer           | 1   | quartz        | $0.70     | $0.70      | https://quartzcomponents.com/products/piezo-buzzer                                                                         |
| pcb                      | 1   | jlcpcb        | $9.00     | $9.00      | https://jlcpcb.com                                                                                                         |
| key and switches         | 1   | unknown       | $0.00     | $0.00      | https://meckeys.com/shop/accessories/keyboard-accessories/key-switches/gateron-5-pin-switch-tester-pack/                   |
| resistor                 | 1   | already owned | $0.00     | $0.00      | https://www.amazon.in/DEPOKE-Value-Resistor-Pack-320pcs/dp/B0CGY55MDP/                                                     |
| leds                     | 1   | robocraze     | $0.30     | $0.30      | https://robocraze.com/products/ws2812-8-bit-rgb-led-straight                                                               |
| Estimated total shipping | 1   |               | $4.00     | $4.00      |                                                                                                                            |
| TOTAL                    |     |               |           | $28.82     |                                                                                                                            |

## Controls

| Input | Action |
|---|---|
| Feed button  | Feed the pet |
| Play button | Play with the pet |
| Shake device | Interact with the pet |

## LED Indicators

| Color | Status |
|---|---|
| Red | Pet is dead |
| Orange | Low hunger |
| Green | High happiness |
| Blue | Normal |

# COMPONENTS

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
thanks to hack club and zach latta

<img width="982" height="531" alt="image" src="https://github.com/user-attachments/assets/3ba23c93-5a16-408c-b6ab-3b3368451368" />

<img width="1077" height="592" alt="Screenshot 2026-10-12 224442" src="https://github.com/user-attachments/assets/6acbe483-669f-43f7-861f-5852875b64e0" />
<img width="1132" height="647" alt="Screenshot 2026-10-12 224349" src="https://github.com/user-attachments/assets/826b3aea-b0a3-476e-b10b-26d8537f2826" />
<img width="1270" height="652" alt="Screenshot 2026-10-12 224324" src="https://github.com/user-attachments/assets/6f82455e-3d2d-49c8-989c-d6ba425b9b09" />
<img width="1050" height="623" alt="Screenshot 2026-10-12 224501" src="https://github.com/user-attachments/assets/c20d8edc-a5fa-4f4c-9874-b131136f5763" />
