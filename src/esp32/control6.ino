#include "I2Cdev.h"  
#include "math.h"

// ===== MOTOR =====
const int PIN_PWMA = 25;
const int PIN_IN1A = 26;
const int PIN_IN2A = 27;
const int PIN_STBY = 14;

// ===== ENCODER =====
const int PIN_ENCA = 5;
const int PIN_ENCB = 4;

// ===== BOTONES =====
const int PIN_BOTON_VERDE = 12;
const int PIN_BOTON_AZUL = 13;

// ===== ULTRASONICOS =====
// Frente
const int PIN_TRIG_FRENTE = 32;
const int PIN_ECHO_FRENTE = 33;

// Derecha
const int PIN_TRIG_DER = 18;
const int PIN_ECHO_DER = 19;

// Izquierda
const int PIN_TRIG_IZQ = 16;
const int PIN_ECHO_IZQ = 17;

#define MAX_DISTANCE 700 
float timeOut = MAX_DISTANCE * 60; 
int soundVelocity = 340; 

// ===== VARIABLES =====
int posEncoder = 0;

int E = 0 , B = 0 , M = 0 , U = 0 , I = 0; 

const int VELOCIDAD_NORMAL = 120;

void setup() {
  Serial.begin(9600);
  setupMotor();
  setupEncoder();
  setupBotones();
  setupUltrasonicos();
}

void loop() {
  desifrarMensaje();
}

void desifrarMensaje(){
  if (Serial.available()) { 
    String mensaje = Serial.readStringUntil('\n'); 
    //Serial.print("Recibido: ");
    //Serial.println(mensaje); 
    if (mensaje[0]=='M'){
      if (mensaje[1]=='0') M=0;
      else if (mensaje[1]=='R') M=-1;
      else M= mensaje[1] - '0';
    } else if (mensaje[0]=='E'){
      if (mensaje[1]=='1') E= 1;
      else {
        resetEncoder();
        E=0;
      } 
    } else if (mensaje[0]=='B'){
      if (mensaje[1]=='1') B= 1;
      else B=0;
    } else if (mensaje[0]=='U'){
      if (mensaje[1]=='1') U= 1;
      else U=0;
    } 
  }

  controlMotor(M);
  if (B == 1) botonesPresionados();
  if (U == 1) mostrarDistancias();
}

// ===== MOTOR =====
void setupMotor() {
  pinMode(PIN_IN1A, OUTPUT);
  pinMode(PIN_IN2A, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, HIGH);
}

void controlMotor(int i) {  
  int velocidad=0;
  switch(i){
    case -1:
      analogWrite(PIN_PWMA, VELOCIDAD_NORMAL);
      digitalWrite(PIN_IN1A, LOW);
      digitalWrite(PIN_IN2A, HIGH);
      return; 
    case 0:
      digitalWrite(PIN_IN1A, LOW);
      digitalWrite(PIN_IN2A, LOW);
      return; 
    case 1: velocidad=100; break;
    case 2: velocidad=130; break;
    case 3: velocidad=160; break;
    case 4: velocidad=190; break;
    case 5: velocidad=220; break;
    case 6: velocidad=255; break;
    default: velocidad=VELOCIDAD_NORMAL; break;
  }
  analogWrite(PIN_PWMA, velocidad);
  digitalWrite(PIN_IN1A, HIGH);
  digitalWrite(PIN_IN2A, LOW);
}

// ===== BOTONES =====
void setupBotones() {
  pinMode(PIN_BOTON_VERDE, INPUT_PULLUP);
  pinMode(PIN_BOTON_AZUL, INPUT_PULLUP);
}

void botonesPresionados(){
  if (BotonVerdePresionado()) {
    Serial.println("BV");
  }
  if (BotonAzulPresionado()) {
    Serial.println("BA");
  }
}

bool BotonVerdePresionado() {
  static int lastStableState = HIGH;
  static int lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;
  
  int reading = digitalRead(PIN_BOTON_VERDE);
  bool buttonPressed = false;
  if (reading != lastButtonState) lastDebounceTime = millis();
  lastButtonState = reading;
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != lastStableState) {
      lastStableState = reading;
      if (lastStableState == LOW) buttonPressed = true;
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
  if (reading != lastButtonState) lastDebounceTime = millis();
  lastButtonState = reading;
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != lastStableState) {
      lastStableState = reading;
      if (lastStableState == LOW) buttonPressed = true;
    }
  }
  return buttonPressed;
}

// ===== ULTRASONICOS =====
void setupUltrasonicos(){
  pinMode(PIN_TRIG_FRENTE,OUTPUT);
  pinMode(PIN_ECHO_FRENTE,INPUT);

  pinMode(PIN_TRIG_DER,OUTPUT);
  pinMode(PIN_ECHO_DER,INPUT);

  pinMode(PIN_TRIG_IZQ,OUTPUT);
  pinMode(PIN_ECHO_IZQ,INPUT);
}

float leerUltrasonico(int trigPin, int echoPin) {
  unsigned long pingTime;
  float distance;
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  pingTime = pulseIn(echoPin, HIGH, timeOut); 
  distance = (float)pingTime * soundVelocity / 2 / 10000; 
  return distance; 
}

void mostrarDistancias(){
  delay(100); 
  float dFrente = leerUltrasonico(PIN_TRIG_FRENTE, PIN_ECHO_FRENTE);
  float dDer = leerUltrasonico(PIN_TRIG_DER, PIN_ECHO_DER);
  float dIzq = leerUltrasonico(PIN_TRIG_IZQ, PIN_ECHO_IZQ);

  //Serial.print("Frente: ");
  //Serial.print(dFrente);
  //Serial.print(" cm | Derecha: ");
  //Serial.print(dDer);
  //Serial.print(" cm | Izquierda: ");
  //Serial.print(dIzq);
  //Serial.println(" cm");
  //Serial.print("F:");
  Serial.print(dFrente, 2);  // 2 decimales
  Serial.print(",");
  Serial.print(dDer, 2);
  Serial.print(",");
  Serial.print(dIzq, 2);   // println cierra la línea
  Serial.print(",");
  Serial.println((float)posEncoder/(float)39);
}

// ===== ENCODER =====
void setupEncoder(){
  pinMode(PIN_ENCA, INPUT);
  pinMode(PIN_ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCA), readEncoder, RISING);
}

void readEncoder(){
  int b= digitalRead(PIN_ENCB);
  if (b>0) posEncoder++;
  else posEncoder--;
}

void resetEncoder(){
  posEncoder=0;
}



