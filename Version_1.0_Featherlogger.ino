// M0 Physics Logger=================================================================
// Using an M0 Adalogger Feather + Digikey Accelerometer + OLED Screen
// This app will write accelerometer data to an SD Card in the Adalogger

//Libraries:=================================================================
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADT7410.h>
#include <Adafruit_ADXL343.h>
#include <SD.h>

//Objects:
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_ADXL343 accel = Adafruit_ADXL343(12345);
Adafruit_ADT7410 tempsensor = Adafruit_ADT7410();


//Digital I/O:
#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5
#define GREENLED  8
#define cardSelect 4


//Variables:
float tempC, accelX, accelY, accelZ;
unsigned long time;
File logfile; 
char filename[] = "LOGGER00.CSV";
boolean loggingpaused = true;


//Setup Routine (Run Once)====================================================
void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.display();
  delay(1000);

  /* Initialise the ADXL343 */
  if (!accel.begin())
  {
    /* There was a problem detecting the ADXL343 ... check your connections */
    Serial.println("Ooops, no ADXL343 detected ... Check your wiring!");
    while (1);
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL343_RANGE_8_G);

  /* Initialise the ADT7410 */
  if (!tempsensor.begin())
  {
    Serial.println("Couldn't find ADT7410!");
    while (1)
      ;
  }

  //Digital IO Defnitions
  pinMode(BUTTON_A, INPUT_PULLUP);              //Buttons on Featherwing
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(GREENLED, OUTPUT);                    //Green LED on Feather Digital 8

  //Get an incremented and new file name to write to
  newfilename();

}
//Subroutines -=======================================================
//Update OLED screen with status
void oled() {
  // text display startup information
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("M0 PyhsicsLogger");
  display.println(" ");
  if (loggingpaused) {
    display.println("Logging: OFF");
    digitalWrite(GREENLED, LOW);                //Also turn Green LED off
  } else {
    display.println("Logging: ON");             //And on when logging
    digitalWrite(GREENLED, HIGH);
  }
  display.print("File: "); display.println(filename); //SD Card filename
  display.setCursor(0, 0);
  display.display();                       // actually display all of the above
}

//Grab the next incremental filename on SD Card and open the file to write
void newfilename() {
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    //error(2);
  }
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)) {          //open new file if it doesn't exist
      break;  // leave the loop!
    }
  }
  logfile = SD.open(filename, FILE_WRITE);
  if ( ! logfile ) {
    Serial.print("Couldnt create ");
    Serial.println(filename);
    //error(3);
  }
  Serial.print("Writing to ");
  Serial.println(filename);

  oled();                                 //Update the Screen
  delay(250);                             //debounce
}

/*
  // blink out an error code
  void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
  }
*/

//Main Loop=====================================================================
void loop() {

  //Grab 3D Acceleration from Sensors
  sensors_event_t event;
  accel.getEvent(&event);
  accelX = event.acceleration.x;
  accelY = event.acceleration.y;
  accelZ = event.acceleration.z;
  tempC = tempsensor.readTempC();

  // Write data to SD Card if Logging is NOT paused
  if (!loggingpaused) {
    logfile.print(millis()), logfile.print(",");
    logfile.print(accelX);  logfile.print(",");
    logfile.print(accelY);  logfile.print(",");
    logfile.print(accelZ);  logfile.print(",");
    logfile.println(tempC);
    
    /*
    Serial.print(millis());  Serial.print(",");
    Serial.print(filename); Serial.print(",");
    Serial.print(accelY);  Serial.print(",");
    Serial.print(accelZ);  Serial.print(",");
    Serial.print(tempC); Serial.print(",");
    if (loggingpaused) {
      Serial.println("Logging Paused");
    } else {
      Serial.println("Logging");
    }
    */
  }

  // if Button A Pressed: Pause/Unpause Logging, Update Screen
  if (!digitalRead(BUTTON_A)) {
    loggingpaused = !loggingpaused;
    oled();                             //Update OLED screen with status
    logfile.flush();                    //flush the file on the SD Card
    //logfile.close();                    //and keep the data safe
    delay(500);                         //Pause to debounce button
  }

  // if Button B Pressed: Close file (if paused), get new file name, open file
  if (!digitalRead(BUTTON_B)) {
    if (loggingpaused) {
      logfile.close();
      delay(250);
      newfilename();                    //Grab next file name
      oled();                           //update status on screen
    }
  }

  //delay(100);   //to slow things down (for use while debugging only)

}
