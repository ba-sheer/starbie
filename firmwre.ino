
#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>


#define OLED_SDA 6
#define OLED_SCL 7

#define DHT_PIN 3
#define DHT_TYPE DHT11

#define BUTTON_FEED 4
#define BUTTON_PLAY 5

#define LED_PIN 21
#define LED_COUNT 3

#define MPU6050_ADDR 0x68

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
    U8G2_R0,
    U8X8_PIN_NONE
);

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_NeoPixel leds(
    LED_COUNT,
    LED_PIN,
    NEO_GRB + NEO_KHZ800
);

Preferences prefs;


struct PetData {
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
unsigned long lastMotionReward = 0;

const unsigned long GAME_TICK = 10000;
const unsigned long DISPLAY_TICK = 150;
const unsigned long SENSOR_TICK = 2500;
const unsigned long MOTION_TICK = 100;
const unsigned long MOTION_REWARD_COOLDOWN = 1000;


bool previousFeed = HIGH;
bool previousPlay = HIGH;


int16_t ax = 0, ay = 0, az = 0;
int16_t gx = 0, gy = 0, gz = 0;

float acceleration = 1.0f;
float temperature = NAN;
float humidity = NAN;


enum PetExpression {
    HAPPY,
    NORMAL,
    SAD,
    SLEEPING,
    DEAD
};

PetExpression expression = NORMAL;

enum Screen {
    MAIN_SCREEN,
    STATUS_SCREEN,
    SENSOR_SCREEN
};

Screen currentScreen = MAIN_SCREEN;


bool initMPU6050() {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x1C);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x1B); 
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    return true;
}

bool readMPU6050() {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x3B);

    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    if (Wire.requestFrom(
            (uint8_t)MPU6050_ADDR,
            (uint8_t)14
        ) != 14) {
        return false;
    }

    ax = (int16_t)((Wire.read() << 8) | Wire.read());
    ay = (int16_t)((Wire.read() << 8) | Wire.read());
    az = (int16_t)((Wire.read() << 8) | Wire.read());

    Wire.read();
    Wire.read();

    gx = (int16_t)((Wire.read() << 8) | Wire.read());
    gy = (int16_t)((Wire.read() << 8) | Wire.read());
    gz = (int16_t)((Wire.read() << 8) | Wire.read());

    float x = ax / 16384.0f;
    float y = ay / 16384.0f;
    float z = az / 16384.0f;

    acceleration = sqrtf(x * x + y * y + z * z);

    return true;
}


void loadPet() {
    prefs.begin("tamagotchi", false);

    pet.hunger = prefs.getInt("hunger", 70);
    pet.happiness = prefs.getInt("happy", 70);
    pet.energy = prefs.getInt("energy", 80);
    pet.health = prefs.getInt("health", 100);

    pet.ageSeconds = prefs.getULong("age", 0);
    pet.alive = prefs.getBool("alive", true);

    pet.hunger = constrain(pet.hunger, 0, 100);
    pet.happiness = constrain(pet.happiness, 0, 100);
    pet.energy = constrain(pet.energy, 0, 100);
    pet.health = constrain(pet.health, 0, 100);
}

void savePet() {
    prefs.putInt("hunger", pet.hunger);
    prefs.putInt("happy", pet.happiness);
    prefs.putInt("energy", pet.energy);
    prefs.putInt("health", pet.health);

    prefs.putULong("age", pet.ageSeconds);
    prefs.putBool("alive", pet.alive);
}

void clampStats() {
    pet.hunger = constrain(pet.hunger, 0, 100);
    pet.happiness = constrain(pet.happiness, 0, 100);
    pet.energy = constrain(pet.energy, 0, 100);
    pet.health = constrain(pet.health, 0, 100);
}


void updateExpression() {
    if (!pet.alive) {
        expression = DEAD;
    } else if (pet.energy < 15) {
        expression = SLEEPING;
    } else if (pet.hunger < 20 || pet.happiness < 20) {
        expression = SAD;
    } else if (pet.happiness > 75) {
        expression = HAPPY;
    } else {
        expression = NORMAL;
    }
}

void updateGame() {
    if (!pet.alive) {
        return;
    }

    pet.ageSeconds += GAME_TICK / 1000UL;

    pet.hunger--;
    pet.happiness--;
    pet.energy--;

    if (pet.hunger < 15) {
        pet.health -= 2;
    }

    if (pet.happiness < 10) {
        pet.health--;
    }

    if (pet.energy < 5) {
        pet.health--;
    }

    clampStats();

    if (pet.health <= 0) {
        pet.health = 0;
        pet.alive = false;
    }

    updateExpression();
    savePet();
}

void feedPet() {
    if (!pet.alive) {
        return;
    }

    pet.hunger += 20;
    pet.happiness += 3;

    clampStats();
    updateExpression();
    savePet();

    Serial.println("Pet fed!");
}

void playWithPet() {
    if (!pet.alive || pet.energy < 10) {
        return;
    }

    pet.happiness += 20;
    pet.energy -= 10;
    pet.hunger -= 5;

    clampStats();
    updateExpression();
    savePet();

    Serial.println("Played with pet!");
}

void checkMotion() {
    if (!pet.alive || !readMPU6050()) {
        return;
    }

    unsigned long now = millis();

    if ((acceleration > 1.35f || acceleration < 0.65f) &&
        now - lastMotionReward >= MOTION_REWARD_COOLDOWN) {

        lastMotionReward = now;

        pet.happiness += 3;
        pet.energy--;

        clampStats();
        updateExpression();
        savePet();
    }
}


void handleButtons() {
    bool feedState = digitalRead(BUTTON_FEED);
    bool playState = digitalRead(BUTTON_PLAY);

    if (previousFeed == HIGH && feedState == LOW) {
        feedPet();
    }

    if (previousPlay == HIGH && playState == LOW) {
        playWithPet();
    }

    previousFeed = feedState;
    previousPlay = playState;
}


void drawPet() {
    oled.drawCircle(64, 35, 22);

    // Ears
    oled.drawLine(48, 20, 43, 13);
    oled.drawLine(43, 13, 51, 17);

    oled.drawLine(80, 20, 85, 13);
    oled.drawLine(85, 13, 77, 17);

    if (expression == SLEEPING) {
        oled.drawLine(52, 32, 58, 32);
        oled.drawLine(70, 32, 76, 32);
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
        oled.drawLine(59, 40, 64, 44);
        oled.drawLine(64, 44, 69, 40);
    } else if (expression == SAD) {
        oled.drawLine(59, 44, 64, 40);
        oled.drawLine(64, 40, 69, 44);
    } else {
        oled.drawLine(61, 41, 67, 41);
    }
}

void drawBar(int x, int y, int width, int height, int value) {
    value = constrain(value, 0, 100);

    oled.drawFrame(x, y, width, height);

    int fill = map(value, 0, 100, 0, width - 2);

    if (fill > 0) {
        oled.drawBox(x + 1, y + 1, fill, height - 2);
    }
}

void drawMainScreen() {
    oled.clearBuffer();
    oled.setFont(u8g2_font_5x7_tf);

    oled.drawStr(2, 7, "TAMAGOTCHI");
    oled.drawStr(105, 7, pet.alive ? "OK" : "RIP");

    drawPet();

    oled.setFont(u8g2_font_4x6_tf);

    oled.drawStr(2, 50, "FOOD");
    drawBar(22, 45, 32, 6, pet.hunger);

    oled.drawStr(2, 61, "HAP");
    drawBar(22, 56, 32, 6, pet.happiness);

    oled.drawStr(68, 50, "ENG");
    drawBar(88, 45, 36, 6, pet.energy);

    oled.drawStr(68, 61, "HP");
    drawBar(88, 56, 36, 6, pet.health);

    oled.sendBuffer();
}

void drawStatusScreen() {
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 10, "PET STATUS");

    oled.setFont(u8g2_font_5x7_tf);

    char buffer[32];

    snprintf(buffer, sizeof(buffer), "Food: %d%%", pet.hunger);
    oled.drawStr(2, 23, buffer);

    snprintf(buffer, sizeof(buffer), "Happy: %d%%", pet.happiness);
    oled.drawStr(2, 33, buffer);

    snprintf(buffer, sizeof(buffer), "Energy: %d%%", pet.energy);
    oled.drawStr(2, 43, buffer);

    snprintf(buffer, sizeof(buffer), "Health: %d%%", pet.health);
    oled.drawStr(2, 53, buffer);

    unsigned long ageHours = pet.ageSeconds / 3600UL;

    snprintf(buffer, sizeof(buffer), "Age: %luh", ageHours);
    oled.drawStr(75, 23, buffer);

    oled.drawStr(75, 53, "FEED: D4");
    oled.drawStr(75, 63, "PLAY: D5");

    oled.sendBuffer();
}

void drawSensorScreen() {
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 10, "SENSORS");

    oled.setFont(u8g2_font_5x7_tf);

    char buffer[32];

    if (!isnan(temperature)) {
        snprintf(buffer, sizeof(buffer), "TEMP: %.1f C", temperature);
        oled.drawStr(2, 25, buffer);
    } else {
        oled.drawStr(2, 25, "TEMP: --");
    }

    if (!isnan(humidity)) {
        snprintf(buffer, sizeof(buffer), "HUM: %.1f %%", humidity);
        oled.drawStr(2, 36, buffer);
    } else {
        oled.drawStr(2, 36, "HUM: --");
    }

    snprintf(buffer, sizeof(buffer), "ACC: %.2f g", acceleration);
    oled.drawStr(2, 47, buffer);

    oled.drawStr(2, 59, "MPU6050 + DHT11");
    oled.sendBuffer();
}


void updateLEDs() {
    uint32_t color;

    if (!pet.alive) {
        color = leds.Color(50, 0, 0);
    } else if (pet.hunger < 20) {
        color = leds.Color(50, 20, 0);
    } else if (pet.happiness > 75) {
        color = leds.Color(0, 50, 10);
    } else {
        color = leds.Color(0, 10, 50);
    }

    for (int i = 0; i < LED_COUNT; i++) {
        leds.setPixelColor(i, color);
    }

    leds.show();
}


void printDebug() {
    Serial.println();
    Serial.println("===== TAMAGOTCHI =====");

    Serial.print("Hunger: ");
    Serial.println(pet.hunger);

    Serial.print("Happiness: ");
    Serial.println(pet.happiness);

    Serial.print("Energy: ");
    Serial.println(pet.energy);

    Serial.print("Health: ");
    Serial.println(pet.health);

    Serial.print("Acceleration: ");
    Serial.println(acceleration);

    Serial.print("Alive: ");
    Serial.println(pet.alive ? "Yes" : "No");

    Serial.println("======================");
}


void setup() {
    Serial.begin(115200);
    delay(500);

    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setClock(400000);

    oled.begin();
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(20, 28, "TAMAGOTCHI");
    oled.drawStr(31, 43, "BOOTING...");
    oled.sendBuffer();

    dht.begin();

    leds.begin();
    leds.clear();
    leds.show();

    pinMode(BUTTON_FEED, INPUT_PULLUP);
    pinMode(BUTTON_PLAY, INPUT_PULLUP);

    bool mpuOK = initMPU6050();

    loadPet();
    updateExpression();
    updateLEDs();

    unsigned long now = millis();
    lastTick = now;
    lastDisplay = now;
    lastSensorRead = now;
    lastMotionRead = now;

    delay(500);

    Serial.println("Tamagotchi firmware started.");
    Serial.print("MPU6050: ");
    Serial.println(mpuOK ? "OK" : "NOT DETECTED");

    printDebug();
}


void loop() {
    unsigned long now = millis();

    handleButtons();

    if (now - lastTick >= GAME_TICK) {
        lastTick = now;
        updateGame();
        updateLEDs();
        printDebug();
    }

    if (now - lastMotionRead >= MOTION_TICK) {
        lastMotionRead = now;
        checkMotion();
    }

    if (now - lastSensorRead >= SENSOR_TICK) {
        lastSensorRead = now;

        temperature = dht.readTemperature();
        humidity = dht.readHumidity();

        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" C | Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
    }

    if (now - lastDisplay >= DISPLAY_TICK) {
        lastDisplay = now;

        switch (currentScreen) {
            case MAIN_SCREEN:
                drawMainScreen();
                break;

            case STATUS_SCREEN:
                drawStatusScreen();
                break;

            case SENSOR_SCREEN:
                drawSensorScreen();
                break;
        }
    }
}
