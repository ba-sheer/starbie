#include<Arduino.h>
#include<Wire.h>
#include<Preferences.h>
#include<U8g2lib.h>
#include<DHT.h>
#include<Adafruit_NeoPixel.h>

#define OLED_SDA 6
#define OLED_SCL 7

#define DHT_PIN 3
#define DHT_TYPE DHT11
#define BUTTON_FEED 4
#define BUTTON_PLAY 5
#define LED_PIN 21
#define LED_COUNT 3

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
    U8G2_R0,
    U8X8_PIN_NONE
);
DHT dht(DHT_PIN,DHT_TYPE);
Adafruit_Neopixel led{
    LED_COUNT,
    LED_PIN,
    LED_GRB + NEO_KHZ800
}

Preferences prefs;
#define MPU6050_ADDR 0x68

int16_t ax,ay,az;
int16_t gx,gy,gz;

float acceleration =0;

struct  PetData
{
    int hunger;
    int happiness;
    int energy;
    int health;

    unsigned long ageSeconds;
    bool alive;
};

PetData pet;

unsigned long lastTick = 0;
unsigned long lastDisplay = 0;
unsigned long lastSensorRead = 0;
unsigned long lastMotionRead = 0;
unsigned long lastFeed =0;
unsigned long lastPlay = 0;

const unsigned long GAME_TICK = 10000;
const unsigned long DISPLAY_TICK = 150;
const unsigned long SENSOR_TICK= 2500;
const unsigned long MOTION_TICK = 100;

bool previousFeed = HIGH;
bool previousPlay = HIGH;

enum PetExpression{
    HAPPY,
    NORMAL,
    SAD,
    SLEEPING,
    DEAD
};
PetExpression expression = NORMAL;
int animationFrame = 0;

enum Screen{
    MAIN_SCREEN,
    STATUS_SCREEN,
    SENSOR_SCREEN
}

Screen currentScreen = MAIN_SCREEN;

bool initMPU6050(){
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x1C);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x1B);
    Wire.write(0x00);
    Wire.endTransmission();
}
bool readMPU6050(){
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x3B);

    if(Wire.endTransmission(false)!=0)
        return false;
    if(Wire.requestFrom(MPU6050_ADDR,14)!=14)
        return false;
    ax = Wire.read()<< 8|Wire.read();
    ay = Wire.read()<< 8|Wire.read();
    az = Wire.read()<< 8|Wire.read();

    Wire.read();
    Wire.read();

    gx =Wire.read()<<8|Wire.read();
    gy= Wire.read()<<8|Wire.read();
    gz = Wire.read()<<8|Wire.read();

    float x=ax/16384.0;
    float y=ay/16384.0;
    float z=az/16384.0;

    acceleration = sqrt(
        x*x +
        y*y +
        z*z
    );
    return true;
}
void loadPet(){
    prefs.begin("tomagotchi",false);

    pet.hunger = prefs.getInt("hunger",70);
    pet.happiness=prefs.getInt("happy",70);
    pet.energy=prefs.getInt("energy",80);
    pet.health=prefs.getInt("health",100);

    pet.ageSeconds = prefs.getULong("age",0);
    pet.alive = prefs.getBool("alive",true);
}
void savePet(){
    prefs.putInt("hunger",pet.hunger);
    prefs.putInt("happy",pet.happiness);
    prefs.putInt("energy",pet.health);
    prefs.putInt("health",pet.health);

    prefs.putULong("age",pet.ageSeconds);
    prefs.putBool("alive",pet.alive);
}
void clampStats(){
    pet.hunger = constrain(pet.hunger,0,100);
    pet.happiness = constrain(pet.happiness,0,100);
    pet.energy = constrain(pet.energy,0,100);
    pet.health = constrain(pet.health,0,100);
}

void updateExpression(){
    if(!pet.alive){
        expression = DEAD;
        return;
    }
    if(pet.energy<15){
        expression = SLEEPING;
        return;
    }
    if(pet.hunger<20||pet.happiness<20){
        expression = SAD;
        return;
    }
    if(pet.happiness>75){
        expression = HAPPY;
        return;
    }
    expression = NORMAL;

}

void updateGame(){
    if(!pet.alive)
    return;
    pet.ageSeconds+=GAME_TICK/1000;
    pet.hunger -= 1;
    pet.happiness -= 1;
    pet.energy -=1;

    if(pet.hunger<15){
        pet.health -=2;
    }
    if(pet.happiness<10){
        pet.health -=1;
    }
    if(pet.energy<5){
        pet.health -=1;
    }
    clampStats();

    if(pet.health <=0){
        pet.health = 0;
        pet.alive = false;
    }
    updateExpression();
    savePet();
}
void feedPet(){
    if(!pet.alive)
    return;
    pet.hunger +=20;
    pet.happiness +=3;
    if(pet.hunger > 100)
    pet.hunger = 100;

    if(pet.happiness>100)
    pet.happiness=100;

    lastFeed=millis();
    savePet();
}
void playWithPet(){
    if(!pet.alive)
    return;
    if(pet,energy <10)
    return;

    pet.happiness+=20;
    pet.energy-=10;
    pet.hunger-=5;

    pet.happiness = constrain(pet.happiness,0,100);
    pet.hunger = constrain(pet.hunger,0,100);
    pet.energy = constrain(pet.energy,0,100);
    lastPlay = millis();
    savePet();
}
void checkMotion(){
    if(!pet.alive)
    return;
    if(!readMPU6050())
    return

    if(acceleration >1.35||acceleration<0.65){
        pet.happiness+=3;
        pet.energy -=1;
        pet.happeness= constrain(
            pet.happiness,
            0,100
        )
        pet.energy = constrain(
            pet.energy,
            0,
            100
        )
        expression = HAPPY;
    }
}

void handleButtons(){
    bool feedState = digitalRead(BUTTON_FEED);
    bool playState = digitalState(BUTTON_PLAY);
    if(previousFeed == HIGH && feedState == LOW){
        feedPet();
    }
    previousFeed = feedState;
    previousPlay = playState;
}
void drawPet() {


    oled.drawCircle(64, 35, 22);

    oled.drawLine(48, 20, 43, 13);
    oled.drawLine(43, 13, 51, 17);

    oled.drawLine(80, 20, 85, 13);
    oled.drawLine(85, 13, 77, 17);

    if (expression == SLEEPING) {

        oled.drawLine(53, 32, 58, 32);
        oled.drawLine(70, 32, 75, 32);

    } else if (expression == DEAD) {

        oled.drawLine(52, 29, 58, 35);
        oled.drawLine(58, 29, 52, 35);

        oled.drawLine(70, 29, 76, 35);
        oled.drawLine(76, 29, 70, 35);

    } else {

        oled.drawDisc(55, 32, 2);
        oled.drawDisc(73, 32, 2);
    }

    if (expression == HAPPY) {

        oled.drawArc(
            64,
            39,
            7,
            7,
            0,
            180
        );

    } else if (expression == SAD) {

        oled.drawArc(
            64,
            46,
            7,
            7,
            180,
            360
        );

    } else {

        oled.drawLine(61, 41, 67, 41);
    }
}
void drawBar(
    int x,
    int y,
    int width,
    int height,
    int value
){
    oled.drawFrame(
        x,
        y,
        width,
        height
    );
    int fill = map(
        value,
        0,
        100,
        0,
        width -2
    );
    oled.drawBox(
        x+1,
        y+1,
        fill,
        height -2
    );
}
void drawMainScreen(){
    oled.clearBuffer();
    oled.setFont(u8g2_font_5x7_tf);
    oled.drawStr(
        2,
        7,
        "TAMAGOTCHI"
    );
    oled.drawStr(
        105,
        7,
        pet.alive?"OK":"RIP"
    );
    drawPet();

    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(2,50,"FOOD");
    drawBar(22,56,32,6,pet.happiness);
    oled.drawStr(2,61,"HAP");
    drawBar(22,56,32,6,pet.happiness);
    oled.drawStr(68,50,"ENG");
    drawBar(88,45,36,6,pet.energy);
    oled.drawStr(68,61 "HP");
    drawBar(88,56,36,6,pet.health);
    oled.sendBuffer();

}
void drawStatusScreen(){
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(
        2,
        10,
        "PET STATUS"
    );
    oled.setFont(u8g2_font_5x7_tf);
    char buffer[32];
    snprint(
        buffer,
        sizeof(buffer),
        "Food:%d%%",
        pet.hunger
    );
    oled.drawStr(,23,buffer);
    snprintf(
        buffer,
        sizeof(buffer),
        "Happy:%d%%",
        pet.happiness
    );
    oled.drawStr(2,33,buffer);
    snprint(
        buffer,
        sizeof(buffer),
        "Energy:%d%%",
        pet.energy
    );
    oled.drawStr(2,43,buffer);
    snprintf(
        buffer,
        sizeof(buffer),
        "Health: %d%%",
        pet.health
    )
    oled.drawStr(2,53,buffer);
    unsigned long ageHours = pet.ageSeconds/3600;
    snprintf(
        buffer,
        sizeof(buffer),
        "Age: %luh",
        ageHours
    );
    oled.drawStr(75,23,buffer);
    oled.drawStr(75,53,"D2:D3");
    oled.sendBuffer();

}
void drawSensorScreen(){
    float temperature =dht.readTemperature();
    float humidity= dht.readHumidity();
    
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(
        2,
        10,
        "SENSORS"
    );
    oled.setFont(u8g2_font_5x7_tf);

    char buffer[32];
    if(!isnan(temperature)){
        snprintf(
            buffer,
            sizeof(buffer),
            "TEMP:%.1f C",
            temperature
        );
        oled.drawStr(2,25,buffer);
    }
    if(!isnan(humidity)){
        snprintf(
            buffer,
            sizeof(buffer),
            "HUM : %.1f %%",
            humidity
        );
        oled.drawStr(2,36,buffer);
    }
    snprintf(
        buffer,
        sizeof(buffer),
        "ACC:%.2f g",
        acceleration
    )
    oled.drawStr(
        2,
        47,
        buffer
    );
    oled.drawStr(
        2,
        59,
        "MPU6050 + DHT11"
    );
    oled.sendBuffer();
}
void updateLEDs(){
    if(!pet.alive){
        for(int i =0;i<LED_COUNT;i++)
        leds.setPixelColor(1,leds.Color(50, 0, 0));
        led.show();
        return;
    }
    if(pet.hunger<20){
        for(int i=0;i<LED_COUNT;i++)
            leds.setPixelColor(
                i,
                leds.Color(50,20,0)
            );
    }else if(pet.happiness>75){
        for(int i =0;i<LED_COUNT;i++)
        leds.setPixelColor(
            i,
            leds.Color(0,50,10)
        );
    }else{
        for(int i = 0;i<LED_COUNT;i++)
        leds.setPixelColor(
            i,
            leds.Color(0,10,50)
        );
    }
    leds.show();
}

void printDebug(){
    Serial.println();
    Serial.println("=====TAMAGOTCHI=====");
    Serial.print("Hunger: ");
    serial.println(pet.hunger);
    serial.print("Happiness: ");
    Serial.println(pet.happiness);
    Serial.print("Energy: ");
    Serial.println(pet.energy);
    Serial.print("Health: ");
    Serial.println(pet.health);
    Serial.print("Acceleration: ");
    Serial.println(acceleration);
    Serial.print("Alive: ");
    Serial.println(pet.alive);
    Serial.println("========================")
}
void setup(){
    Serial.begin(115200);

    delay(500);

    // I2C
    Wire.begin(
        OLED_SDA,
        OLED_SCL
    );

    Wire.setClock(400000);

    // OLED
    oled.begin();

    oled.clearBuffer();

    oled.setFont(
        u8g2_font_6x10_tf
    );

    oled.drawStr(
        20,
        28,
        "TAMAGOTCHI"
    );

    oled.drawStr(
        31,
        43,
        "BOOTING..."
    );

    oled.sendBuffer();

    // DHT
    dht.begin();

    // LEDs
    leds.begin();
    leds.clear();
    leds.show();

    // Buttons
    pinMode(
        BUTTON_FEED,
        INPUT_PULLUP
    );

    pinMode(
        BUTTON_PLAY,
        INPUT_PULLUP
    );

    // MPU6050
    initMPU6050();

    loadPet();

    updateExpression();

    delay(1000);

    Serial.println(
        "Tamagotchi firmware started."
    );

    printDebug();
}
void loop(){
    unsigned long now = millis();

    // Buttons
    handleButtons();

    // Game mechanics
    if (now - lastTick >= GAME_TICK) {

        lastTick = now;

        updateGame();
        updateLEDs();

        printDebug();
    }

    // Motion
    if (now - lastMotionRead >= MOTION_TICK) {

        lastMotionRead = now;

        checkMotion();
    }

    // Screen
    if (now - lastDisplay >= DISPLAY_TICK) {

        lastDisplay = now;

        if (currentScreen == MAIN_SCREEN) {

            drawMainScreen();

        } else if (currentScreen == STATUS_SCREEN) {

            drawStatusScreen();

        } else {

            drawSensorScreen();
        }
    }

    // Long-term sensor sampling
    if (now - lastSensorRead >= SENSOR_TICK) {

        lastSensorRead = now;

        float temperature =
            dht.readTemperature();

        float humidity =
            dht.readHumidity();

        Serial.print("Temperature: ");
        Serial.print(temperature);

        Serial.print(" C | Humidity: ");
        Serial.print(humidity);

        Serial.println(" %");
    }
}