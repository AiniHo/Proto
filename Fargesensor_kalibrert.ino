#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>


#define LED_PIN 5
#define LED_COUNT 7

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGBW + NEO_KHZ800);

 
// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;
 
void setup() {
  strip.begin();           
  strip.show();            
  strip.setBrightness(255); 
  
  // Initialize Serial port
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("--------------------------------"));
  Serial.println(F("APDS-9960 - ColorSensor"));
  Serial.println(F("--------------------------------"));
  
  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Start running the APDS-9960 light sensor (no interrupts)
  if ( apds.enableLightSensor(false) ) {
    Serial.println(F("Light sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during light sensor init!"));
  }
  
  // Wait for initialization and calibration to finish
  delay(500);
}
 
void loop() {
  for(int i = 0; i < strip.numPixels(); i++){
    strip.setPixelColor(i, 255, 255, 255, 255);
    strip.show();
    delay(40); 
  }
  
  // Read the light levels (ambient, red, green, blue)
  if (  !apds.readAmbientLight(ambient_light) ||
        !apds.readRedLight(red_light) ||
        !apds.readGreenLight(green_light) ||
        !apds.readBlueLight(blue_light) ) {
    Serial.println("Error reading light values");
  } 
  else {
  
    Serial.print("Ambient: ");
    Serial.print(ambient_light);
    Serial.print(" Red: ");
    Serial.print(red_light);
    Serial.print(" Green: ");
    Serial.print(green_light);
    Serial.print(" Blue: ");
    Serial.println(blue_light);

    if (red_light > 300 && red_light < 500  &&  green_light > 300 && green_light < 600   &&  blue_light > 300 && blue_light < 500)  Serial.println("color = YELLOW");
    else if (red_light > 250 && red_light < 400   &&  green_light > 0 && green_light < 150    &&  blue_light > 50 && blue_light < 200)   Serial.println("color = ORANGE");
    else if (red_light > 0 && red_light < 350   &&  green_light > 50 && green_light < 250    &&  blue_light > 200 && blue_light < 500)   Serial.println("color = BLUE");
    else if (red_light > 100  && red_light < 200   &&  green_light >  0 && green_light < 50    &&  blue_light > 0  && blue_light < 100)   Serial.println("color = RED") ;
    else if (red_light > 100 && red_light < 200   &&  green_light > 100 && green_light < 180    &&  blue_light > 100 && blue_light < 150)   Serial.println("color = PURPLE");
    else  Serial.println("color = NO COLOR");

  }
  // Wait 1 second before next reading
  delay(1000);
}