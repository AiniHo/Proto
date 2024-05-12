#include <Servo.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

//Ledring
#define LED_PIN_RING 10
#define LED_COUNT_12 12
Adafruit_NeoPixel ring(LED_COUNT_12, LED_PIN_RING, NEO_RGBW + NEO_KHZ800);

//SensorLed
#define LED_PIN_SENSOR 5
#define LED_COUNT_3 3
Adafruit_NeoPixel sensorLed(LED_COUNT_3, LED_PIN_SENSOR, NEO_RGBW + NEO_KHZ800);

// Globale variabler for fargesensor
SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;

Servo vektServo;
const int buzzer = 7;

const int magSensor = A0;
int magSensorVal = 0;

//Struct for å matche fargeavlesning til planetfarge
struct Color {
  int red_min;
  int red_max;
  int green_min;
  int green_max;
  int blue_min;
  int blue_max;
  int ambient_min;
  int ambient_max;
};

// Definere fargeintervallene fra kalibrering
Color plutoColor = { 1000, 1300, 1600, 2100, 2700, 3800, 0, 0 };    //lilla
Color merkurColor = { 1550, 1850, 1150, 1300, 2100, 2300, 0, 0 };   //rød
Color jordaColor = { 700, 900, 1700, 2300, 3000, 4500, 0, 0 };      //blå
Color jupiterColor = { 1750, 2000, 1200, 1400, 2100, 2400, 0, 0 };  //oransje
Color solaColor = { 1250, 1700, 1800, 2400, 2400, 2900, 0, 0 };     //gul

//Struct for rgb verdier til ledstripe
struct RGB {
  int red;
  int green;
  int blue;
};

//Definerer fargene til ledstripen
RGB yellow = { 255, 255, 0 };
RGB blue = { 0, 0, 255 };
RGB orange = { 255, 165, 0 };  // Adjust values for orange
RGB red = { 255, 0, 0 };
RGB purple = { 128, 0, 128 };  // Adjust values for purple


struct Planet {
  String name;       // Name of the planet
  int angle;         // Angle of the servo corresponding to this planet
  String colorName;  //Fargenavn
  Color colorrange;  //Fargerange
  double speed;      // Speed of the planet (in milliseconds)
  RGB ledColor;      //color for the LED
};

// Definere informasjon for fem planeter
Planet planets[5] = {
  { "Pluto", 0, "Purple", plutoColor, 200, purple },         // Name and angle for planet 1
  { "Merkur", 40, "Red", merkurColor, 500, red },            // Name and angle for planet 2
  { "Jorda", 80, "Blue", jordaColor, 1000, blue },           // Name and angle for planet 3
  { "Jupiter", 125, "Orange", jupiterColor, 2000, orange },  // Name and angle for planet 4
  { "Sola", 179, "Yellow", solaColor, 3000, yellow }         // Name and angle for planet 5
};

void setup() {

  vektServo.attach(9);
  pinMode(buzzer, OUTPUT);
  pinMode(magSensor, INPUT);

  Serial.begin(9600);

  ring.begin();
  ring.show();
  ring.setBrightness(255);

  sensorLed.begin();
  sensorLed.show();
  sensorLed.setBrightness(255);

  //starter opp apds9960
  if (!apds.init()) {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  //starter fargesensor
  if (!apds.enableLightSensor(false)) {
    Serial.println(F("Something went wrong during light sensor init!"));
  }
  //Delay for å kalibrere fargesensor
  delay(500);
}


void loop() {

  bool magnetDetected = false;

  magSensorVal = analogRead(magSensor);
  delay(1000);
  Serial.print("Sensor value: ");
  Serial.println(magSensorVal);
  magSensorVal = map(magSensorVal, 480, 500, 0, 10);
  Serial.println(magSensorVal);

  // Check if a magnet is detected
  magnetDetected = ((magSensorVal >= 8 && magSensorVal != 7) || magSensorVal <= 4);

  while (magnetDetected) {

    Serial.println("Magnet detected. Sensor val: ");
    Serial.println(magSensorVal);
    //Turn on color light sensor
    sensorLedOn();

    // Les farge fra sensor
    if (readColor()) {

      Serial.println("Fargeavlesning");

      apds.readRedLight(red_light);
      apds.readAmbientLight(ambient_light);
      apds.readGreenLight(green_light);
      apds.readBlueLight(blue_light);

      // Matcher planet til indeks in listen med planetene
      int matchedPlanetIndex = matchColorToPlanet(red_light, green_light, blue_light, ambient_light);
      Serial.println(matchedPlanetIndex);

      //hvis en matchende planet er funnet
      if (matchedPlanetIndex != -1) {
        // Move servo to the angle of the matched planet
        Serial.println(planets[matchedPlanetIndex].name);
        Serial.println(planets[matchedPlanetIndex].colorName);

        //Setter servo til tilsvarende vinkel
        vektServo.write(planets[matchedPlanetIndex].angle);

        //Piezo lager pip
        tone(buzzer, 1000);
        delay(100);
        noTone(buzzer);
        runLedAnimation(planets[matchedPlanetIndex], ring);  //Animerer ringled
      } else {
        delay(1000);
      }
    }
  }
  //Skrur sensorLed av når planet ikke er detektert
  sensorLedOff();
  vektServo.write(0);
  Serial.println(magSensorVal);
}

// Funksjon som leser farger fra sensor
bool readColor() {
  if (!apds.readAmbientLight(ambient_light) || !apds.readRedLight(red_light) || !apds.readGreenLight(green_light) || !apds.readBlueLight(blue_light)) {
    Serial.println("Error reading light values");
    return false;
  }
  return true;
}

// Funksjon som finner indeksen til planeten med data fra fargesensor
int matchColorToPlanet(uint16_t red_light, uint16_t green_light, uint16_t blue_light, uint16_t ambient_light) {

  for (int i = 0; i < 5; i++) {
    if (red_light >= planets[i].colorrange.red_min && red_light <= planets[i].colorrange.red_max
        && green_light >= planets[i].colorrange.green_min && green_light <= planets[i].colorrange.green_max
        && blue_light >= planets[i].colorrange.blue_min && blue_light <= planets[i].colorrange.blue_max) {
      return i;  // Return the index of the matched planet
    }
  }
  return -1;  // Return -1 hvis ingen planeter passer med avlesning
}


//Animerer ringled med rundetid av/på tilsvarende vaiablene speed til planeten
void runLedAnimation(Planet planet, Adafruit_NeoPixel& ledStrip) {
  // Calculate the delay for each step based on the planet's speed
  int stepDelay = planet.speed / (2 * ring.numPixels());  // Divide by 2 for both activation and deactivation

  // Display the ring pattern with the specified RGB color
  for (int i = 0; i < ring.numPixels(); i++) {
    // Turn on LEDs gradually
    ledStrip.setPixelColor(i, planet.ledColor.red, planet.ledColor.green, planet.ledColor.blue, 0);
    ledStrip.show();
    delay(stepDelay);
  }

  // Turn off the LEDs gradually
  for (int i = ring.numPixels() - 1; i >= 0; i--) {
    ledStrip.setPixelColor(i, 0, 0, 0, 0);
    ledStrip.show();
    delay(stepDelay);
  }
}


void sensorLedOn() {
  sensorLed.setPixelColor(0, 255, 255, 255, 255);
  sensorLed.setPixelColor(1, 255, 255, 255, 255);
  sensorLed.setPixelColor(2, 255, 255, 255, 255);
  sensorLed.show();
}

void sensorLedOff() {
  sensorLed.clear();
  sensorLed.show();
}

// Takk til Sparkfun
//Takk til ADAFRUIT
// Takk til studassene <3

//Takk for oss!
