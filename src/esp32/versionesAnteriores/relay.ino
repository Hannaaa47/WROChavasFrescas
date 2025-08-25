#include <XboxSeriesXControllerESP32_asukiaaa.hpp>
//servo
#include <ESP32Servo.h>
#define ADC_Max 65535    // This is the default ADC max value on the ESP32 (12 bit ADC width); 

// Required to replace with your xbox address
// XboxSeriesXControllerESP32_asukiaaa::Core
// xboxController("44:16:22:5e:b2:d4");
// xboxController("44:16:22:71:c4:2d");


// any xbox controller
XboxSeriesXControllerESP32_asukiaaa::Core xboxController;

//servo
Servo myservo;  // create servo object to control a servo

int servoPin = 15;      // GPIO pin used to connect the servo control (digital out)
int potPin = 34;        // GPIO pin used to connect the potentiometer (analog in)
int potVal;             //variable to read the value from the analog pin
int motorsin = 14;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting NimBLE Client");
  xboxController.begin();

  //servo
  myservo.setPeriodHertz(50);// Standard 50hz servo
  // attaches the servo on servoPin to the servo object
  myservo.attach(servoPin, 500, 2500);
  Serial.begin(115200);

  //motor
  pinMode(motorsin, OUTPUT);

}

void loop() {
  xboxController.onLoop();
  if (xboxController.isConnected()) {
    if (xboxController.isWaitingForFirstNotification()) {
      Serial.println("waiting for first notification");
    } else {
      Serial.println("Address: " + xboxController.buildDeviceAddressStr());
      Serial.print(xboxController.xboxNotif.toString());
      unsigned long receivedAt = xboxController.getReceiveNotificationAt();
      uint16_t joystickMax = XboxControllerNotificationParser::maxJoy;
      Serial.print("joyLHori rate: ");
      Serial.println((float)xboxController.xboxNotif.joyLHori / joystickMax);
      Serial.print("joyLVert rate: ");
      Serial.println((float)xboxController.xboxNotif.joyLVert / joystickMax);
      Serial.println("battery " + String(xboxController.battery) + "%");
      Serial.println("received at " + String(receivedAt));

      //servo
      // read the value of the potentiometer (value between 0 and 4095)
      potVal = xboxController.xboxNotif.joyLHori;
      Serial.printf("potVal_1: %d\t", potVal);
      // scale it to use it with the servo (value between 0 and 180)
      potVal = map(potVal, 0, ADC_Max, 0, 180);
      // set the servo position according to the scaled value
      myservo.write(potVal);
      Serial.printf("potVal_2: %d\n", potVal);
      delay(15);// wait for the servo to get there

      if (xboxController.xboxNotif.btnA == true) {
        digitalWrite(motorsin, LOW);
      } else {
        digitalWrite(motorsin, HIGH);
      }


    }
  } else {
    Serial.println("not connected");
    digitalWrite(motorsin, HIGH);
    if (xboxController.getCountFailedConnection() > 2) {
      ESP.restart();
    }
  }
  Serial.println("at " + String(millis()));
  delay(500);
}
