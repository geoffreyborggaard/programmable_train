//motor A connected between A01 and A02
//motor B connected between B01 and B02
#include <stdarg.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#include "Adafruit_TCS34725.h"


#define commonAnode true

const int NO_COLOR = -1;

const int WHT = 0;
const int RED = 1;
const int GREEN = 2;
const int BLUE = 3;
const int BLK = 4;

int redPin = 3;  // red RGB LED
int greenPin = 5;  // green RGB LED
int bluePin = 6;  // blue RGB LED

int STBY = 3; //standby

//Motor A
int PWMA = 9; //Speed control 
int AIN1 = 7; //Direction
int AIN2 = 8; //Direction

int power = 110;

int direction = 0;


// our RGB -> eye-recognized gamma color
byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 10, NEO_GRB + NEO_KHZ800);


void p(char *fmt, ... );

int lastColor = NO_COLOR;
int colorCount = 0;

void loop(){    
  int color = determineColor();
  if (color == lastColor) {
    colorCount++;
  } else {
    lastColor = color;
    colorCount = 0;
  }
  
  if (colorCount == 5) {
    switch (color) {
      case RED:   
        Serial.println("Red");
        direction = 1;
        move(power); //motor 1, full speed, forward.
        break;
      case BLUE:
         Serial.println("blue");
        break;
      case GREEN:
        Serial.println("green");
        direction = 0;
        move(power); //motor 1, half speed, black.
        move(0);
        break;
      case WHT:
        Serial.println("white");
        direction = (direction == 1) ? 0 : 1; // Reverse direction.
        break;
      case BLK:
        Serial.println("black");
        break;
       default:
       break;
    }
  }
}



void p(char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}


void setup(){
  pinMode(STBY, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);


  strip.begin();
  strip.show();

  /* start up serial at 9600 baud, and print a welcome message */
  Serial.begin(9600);
  Serial.println("Hello world, I'm Thomas!");

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }
    // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;      
    }
  }
}

int determineColor() {
    uint16_t clear, red, green, blue;

  tcs.setInterrupt(false);      // turn on LED

  delay(60);  // takes 50ms to read 
  
  tcs.getRawData(&red, &green, &blue, &clear);

  tcs.setInterrupt(true);  // turn off LED
 
  // Figure out some basic hex code for visualization
  uint32_t sum = red;
  sum += green;
  sum += blue;
  sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  
  
  /*Serial.print("C:\t"); Serial.print(clear);
  Serial.print("\tR:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.print(blue);
*/
  Serial.print("\t");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();

  //strip.setPixelColor(0, r, g, b); 

  
  if (clear < 6000) {
    strip.setPixelColor(0, 0, 0, 0);
    return BLK;
  } else if (r > 240 && g > 240 && b > 0xD0) {
    strip.setPixelColor(0, 150, 150, 150);
    return WHT;
  } else if (r > 0x90) {
    strip.setPixelColor(0, 255, 0, 0);
    return RED;
  } else if (b > 0x70) {
    strip.setPixelColor(0, 0, 0, 255);
    return BLUE;
  } else if (g > 0x70) {
    strip.setPixelColor(0, 0, 255, 0);
    return GREEN;
  }

  strip.show();

  return NO_COLOR;  
}

int setColor(int red, int green, int blue) {
    analogWrite(redPin, 255-red);
    analogWrite(greenPin, 255-green);
    analogWrite(bluePin, 255-blue);
}

void move(int speed){
//Move specific motor at speed and direction
//motor: 0 for B 1 for A
//speed: 0 is off, and 255 is full speed
//direction: 0 clockwise, 1 counter-clockwise

  digitalWrite(STBY, HIGH); //disable standby

  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;

  if(direction == 1){
    inPin1 = HIGH;
    inPin2 = LOW;
  }

    digitalWrite(AIN1, inPin1);
    digitalWrite(AIN2, inPin2);
    analogWrite(PWMA, speed);
}

void stop(){
//enable standby  
  digitalWrite(STBY, LOW); 
}

