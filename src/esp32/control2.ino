#include <Wire.h>
#include <MPU6050.h>  
#include "I2Cdev.h"  
#include "math.h"

//IMU
MPU6050 accelgyro;

int16_t ax, ay, az;  
int16_t gx, gy, gz;
double angleA1, angleA2; 
double angleG1, angleG2; 
double angleC1, angleC2; 

//MOTOR
const int PIN_PWMA = 25;
const int PIN_IN1A = 27;
const int PIN_IN2A = 26;
const int PIN_STBY = 14;

//ENCODER LO VOY DEJAR ASI NOMAS POR MIENTRAS
const int PIN_ENCA = 5;
const int PIN_ENCB = 4;

//BOTONES
const int PIN_BOTON_VERDE = 12;
const int PIN_BOTON_AZUL = 13;

//ULTRASONICO
const int PIN_TRIG = 32; // define PIN_TRIG
const int PIN_ECHO = 33; // define PIN_ECHO.
#define MAX_DISTANCE 700 // Maximum sensor distance is rated at 400-500cm.
//timeOut= 2*MAX_DISTANCE /100 /340 *1000000 = MAX_DISTANCE*58.8
float timeOut = MAX_DISTANCE * 60; 
int soundVelocity = 340; // define sound speed=340m/s


int posEncoder = 0; //posicion del encoder

int E = 0 , B = 0 , M = 0 , U = 0 , I = 0 , S = 0; //valores de los mensajes


const int VELOCIDAD_NORMAL = 250;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupMotor();
  setupEncoder();
  setupBotones();
  setupUltrasonico();
  setupIMU();
}

void loop() {
  desifrarMensaje();
}

void desifrarMensaje(){
  // Mensajes           Pueden estar en true o false
  //                                     1      0
  //  Encoder       E
  //  Botones       B
  //  Motor         M
  //  IMU           I
  //  Ultrasonico   U
  if (Serial.available()) { // Si hay datos recibidos
    String mensaje = Serial.readStringUntil('\n'); // Lee hasta salto de línea
    //Serial.print("Recibido: ");
    //Serial.println(mensaje); // Responde por serial
    if (mensaje[0]=='M'){
      if (mensaje[1]=='1') {
        M=1;
        //Serial.println("Motor avanza");
      }
      else M=0;
    } else if (mensaje[0]=='E'){
      if (mensaje[1]=='1') E= 1;
      else E=0;
    } else if (mensaje[0]=='B'){
      if (mensaje[1]=='1') B= 1;
      else B=0;
    } else if (mensaje[0]=='S'){
      if (mensaje[1]=='1') S= 1;
      else S=0;
    } else if (mensaje[0]=='U'){
      if (mensaje[1]=='1') U= 1;
      else U=0;
    } else if (mensaje[0]=='I'){
      if (mensaje[1]=='1') I= 1;
      else I=0;
    }
  }

  if (M == 1){
    avanzar();
  } else if (M == 0){
    detenerse();
  }
  if (E == 1) imprimePosEncoder();
  if (B == 1) botonesPresionados();
  if (U == 1) mostrarDistancia();
  if (I == 1) {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    anguloGiroscopio();
    delay(100);  
  }

}

//MOTOR
void setupMotor() {
  pinMode(PIN_IN1A, OUTPUT);
  pinMode(PIN_IN2A, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, HIGH);
}

void setMotor(int dir, int pwmVal) {
  analogWrite(PIN_PWMA, pwmVal);
  if (dir == 1) {
    digitalWrite(PIN_IN1A, HIGH);
    digitalWrite(PIN_IN2A, LOW);
  } else if (dir == -1) {
    digitalWrite(PIN_IN1A, LOW);
    digitalWrite(PIN_IN2A, HIGH);
  } else {
    digitalWrite(PIN_IN1A, LOW);
    digitalWrite(PIN_IN2A, LOW);
    //digitalWrite(PIN_STBY, LOW);
  }
}

void avanzar() {
  analogWrite(PIN_PWMA, VELOCIDAD_NORMAL);
  digitalWrite(PIN_IN1A, HIGH);
  digitalWrite(PIN_IN2A, LOW);
}

void reversa() {
  analogWrite(PIN_PWMA, VELOCIDAD_NORMAL);
  digitalWrite(PIN_IN1A, LOW);
  digitalWrite(PIN_IN2A, HIGH);
}

void detenerse() {
  digitalWrite(PIN_IN1A, LOW);
  digitalWrite(PIN_IN2A, LOW);
}


//ENCODER
void setupEncoder(){
  pinMode(PIN_ENCA, INPUT);
  pinMode(PIN_ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCA), readEncoder, RISING);
}

void readEncoder(){
  int b= digitalRead(PIN_ENCB);
  if (b>0){
    posEncoder++;
  } else {
    posEncoder--;
  }
}

void imprimePosEncoder(){
  Serial.println(posEncoder);
}

//BOTONES
void setupBotones() {
  pinMode(PIN_BOTON_VERDE, INPUT_PULLUP);
  pinMode(PIN_BOTON_AZUL, INPUT_PULLUP);
}

void botonesPresionados(){
  if (BotonVerdePresionado()) {
    Serial.println("Boton verde presionado");
  }
  if (BotonAzulPresionado()) {
    Serial.println("Boton azul presionado");
  }
}

bool BotonVerdePresionado() {
  static int lastStableState = HIGH;
  static int lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;
  
  int reading = digitalRead(PIN_BOTON_VERDE);
  bool buttonPressed = false;
  
  // Verificar si el estado cambió (ruido o pulsación real)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // Actualizar el último estado
  lastButtonState = reading;
  
  // Si ha pasado el tiempo de debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Verificar si el estado estable cambió
    if (reading != lastStableState) {
      lastStableState = reading;
      
      // Detectar flanco de bajada (botón presionado)
      if (lastStableState == LOW) {
        buttonPressed = true;
      }
    }
  }
  
  return buttonPressed;
}

bool BotonAzulPresionado() {
  static int lastStableState = HIGH;
  static int lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;
  
  int reading = digitalRead(PIN_BOTON_AZUL);
  bool buttonPressed = false;
  
  // Verificar si el estado cambió (ruido o pulsación real)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // Actualizar el último estado
  lastButtonState = reading;
  
  // Si ha pasado el tiempo de debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Verificar si el estado estable cambió
    if (reading != lastStableState) {
      lastStableState = reading;
      
      // Detectar flanco de bajada (botón presionado)
      if (lastStableState == LOW) {
        buttonPressed = true;
      }
    }
  }
  
  return buttonPressed;
}

void mostrarDistancia(){
  delay(100); // Wait 100ms between pings (about 20 pings/sec).
  //Serial.print("Ultrasonico: "); 
  //Serial.printf("Distance -> ");
  Serial.println(UltrasonicoLoop()); // Send ping, get distance in cm and print result 
  //Serial.println("cm");
}

void setupUltrasonico(){
  pinMode(PIN_TRIG,OUTPUT);// set PIN_TRIG to output mode
  pinMode(PIN_ECHO,INPUT); // set PIN_ECHO to input mode
}

float UltrasonicoLoop() {
  unsigned long pingTime;
  float distance;
  // make PIN_TRIG output high level lasting for 10μs to triger HC_SR04
  digitalWrite(PIN_TRIG, HIGH); 
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  // Wait HC-SR04 returning to the high level and measure out this waitting time
  pingTime = pulseIn(PIN_ECHO, HIGH, timeOut); 
  // calculate the distance according to the time
  distance = (float)pingTime * soundVelocity / 2 / 10000; 
  return distance; // return the distance value
}

void setupIMU(){
  Wire.begin();   
  accelgyro.initialize();
}


void anguloGiroscopio(){
  angleG1 = angleG1+float(gy)*0.01/131;
  angleG2 = angleG2+float(gx)*0.01/131;

  Serial.print("Anglex :");
  Serial.print(angleG2); 
  Serial.print("\t");
  Serial.print("Angley :");
  Serial.println(angleG1); 
  Serial.print("\t");
}
