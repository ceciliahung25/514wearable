#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

void displayMessage(const char* message, bool isMotion, float temperature, float humidity);
void drawSmileyFace(int x, int y, bool smiling);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define BME_SDA 4
#define BME_SCL 5
#define VIBRATION_PIN 6

#define OLED_RESET -1 // 不使用 reset 引脚
#define SCREEN_ADDRESS 0x3C // I2C 地址

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MPU6050 mpu;
Adafruit_BME280 bme;

const int numReadings = 10;
float tempReadings[numReadings];
float humReadings[numReadings];
float accReadings[numReadings];
int readIndex = 0;
float totalTemp = 0;
float totalHum = 0;
float totalAcc = 0;
float averageTemp = 0;
float averageHum = 0;
float averageAcc = 0;

void setup() {
    Serial.begin(9600);

    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050");
        while (1);
    }
    Serial.println("MPU6050 Found!");

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.display();
    delay(2000);
    display.clearDisplay();

    if (!bme.begin(0x76)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    pinMode(VIBRATION_PIN, OUTPUT);

    for (int i = 0; i < numReadings; i++) {
        tempReadings[i] = 0;
        humReadings[i] = 0;
        accReadings[i] = 0;
    }

    Serial.println("Setup complete!");
}

void loop() {
    sensors_event_t a, g, tempEvent;
    mpu.getEvent(&a, &g, &tempEvent);

    float newTemp = bme.readTemperature();
    float newHum = bme.readHumidity();
    float acceleration = sqrt(a.acceleration.x * a.acceleration.x + 
                              a.acceleration.y * a.acceleration.y + 
                              a.acceleration.z * a.acceleration.z);

    // 更新滤波前的读数
    Serial.print("Raw Temp: "); Serial.print(newTemp);
    Serial.print(" C, Raw Hum: "); Serial.print(newHum);
    Serial.println(" %");

    totalTemp = totalTemp - tempReadings[readIndex];
    totalHum = totalHum - humReadings[readIndex];
    totalAcc = totalAcc - accReadings[readIndex];

    tempReadings[readIndex] = newTemp;
    humReadings[readIndex] = newHum;
    accReadings[readIndex] = acceleration;

    totalTemp = totalTemp + tempReadings[readIndex];
    totalHum = totalHum + humReadings[readIndex];
    totalAcc = totalAcc + accReadings[readIndex];

    readIndex = (readIndex + 1) % numReadings;

    averageTemp = totalTemp / numReadings;
    averageHum = totalHum / numReadings;
    averageAcc = totalAcc / numReadings;

    // 打印滤波后的平均读数
    Serial.print("Filtered Avg Temp: "); Serial.print(averageTemp);
    Serial.print(" C, Filtered Avg Hum: "); Serial.print(averageHum);
    Serial.println(" %");

    if (averageAcc > 10.0) {
        digitalWrite(VIBRATION_PIN, HIGH);
        delay(1000);
        digitalWrite(VIBRATION_PIN, LOW);
    }

    displayMessage("Let's Practice!", averageAcc > 10.0, averageTemp, averageHum);
    Serial.print("\"Raw Accel\": ");
    Serial.print(acceleration);
    Serial.print(", \"Filtered Accel\": ");
    Serial.println(averageAcc);

    delay(1000); // 根据需要调整此延时
}

void displayMessage(const char* message, bool isMotion, float temperature, float humidity) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.setCursor(0, 10);
    display.print("Hum: ");
    display.print(humidity);
    display.println(" %");

    display.setCursor(15, 20);
    display.println(isMotion ? "Let's Practice!" : "Rest, Savasana...");

    drawSmileyFace(0, 20, isMotion);
    display.display();
}

void drawSmileyFace(int x, int y, bool smiling) {
    display.drawCircle(x + 4, y + 3, 3, SSD1306_WHITE);
    display.drawPixel(x + 3, y + 2, SSD1306_WHITE);
    display.drawPixel(x + 5, y + 2, SSD1306_WHITE);
    if (smiling) {
        display.drawPixel(x + 2, y + 4, SSD1306_WHITE);
        display.drawPixel(x + 3, y + 5, SSD1306_WHITE);
        display.drawPixel(x + 4, y + 5, SSD1306_WHITE);
        display.drawPixel(x + 5, y + 4, SSD1306_WHITE);
    } else {
        display.drawPixel(x + 2, y + 5, SSD1306_WHITE);
        display.drawPixel(x + 3, y + 4, SSD1306_WHITE);
        display.drawPixel(x + 4, y + 4, SSD1306_WHITE);
        display.drawPixel(x + 5, y + 5, SSD1306_WHITE);
    }
}
