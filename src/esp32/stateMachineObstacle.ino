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
float dEncoder = 0;
float dFrente = 0, dDer = 0, dIzq = 0;
float dfFrente = 0, dfIzq = 0, dfDer = 0;
float alpha = 0.7;
bool BV = false, BA = false;
int E = 0, B = 0, M = 0, U = 0, I = 0;
const int VELOCIDAD_NORMAL = 120;
char obstaculo = 'Y';

// -------------- Variables state machine
enum State { INICIO, AVANZAR_ABIERTA, SENTIDO_ABIERTA, GIRAR_ABIERTA, AVANCE_FINAL_ABIERTA, DETENERSE_ABIERTA, 
                    AVANZAR_OBSTACULOS, SENTIDO_OBSTACULOS, GIRAR_OBSTACULOS, ESQUIVA_OBSTACULOS,  AVANCE_FINAL_OBSTACULOS, DETENERSE_OBSTACULOS};

State currentState = INICIO;

//variables de la prueba
int distance = 100000;
int vueltas = 0;
int turns = 12;
unsigned long t_s1_v = 0;
unsigned long t_0_s1_v = 0;
int vuelta_delay = 5000;
int velocidad = 150;
bool fin = false;
int sentido = -1;

void setup() {
  Serial.begin(9600);
  setupMotor();
  setupEncoder();
  setupBotones();
  setupUltrasonicos();
}

void loop() {
  stateMachine();
  actualizarDistancias();
}

void stateMachine() {
  switch (currentState) {
    case INICIO:
      BotonVerdePresionado();
      BotonAzulPresionado();
      if (BV) {
        girarServo(1);
        delay(4000);
        girarServo(0);
        delay(4000);
        currentState = AVANZAR_ABIERTA;
        Serial.println("AVANZA_ABIERTA");
        resetEncoder();
      } else if (BA){
        girarServo(1);
        delay(4000);
        girarServo(0);
        delay(4000);
        currentState = AVANZAR_OBSTACULOS;
        Serial.println("AVANZAR_OBSTACULOS");
        resetEncoder();
      }
      break;

    case AVANZAR_ABIERTA:
      controlMotor(1);
      if (vueltas != 0) {
        if (sentido == -1) {
          if (dfDer > 30) {
            Serial.println("corrige derecha");
            girarServo(2);
            delay(100);
          }
          else if (dfDer < 20) {
            Serial.println("corrige izquierda");
            girarServo(-2);
            delay(100);
          } else {
            girarServo(0);
            delay(100);
          }
        } else {
          if (dfIzq > 30) {
            Serial.println("corrige izquierda");
            girarServo(-2);
          } else if (dfIzq < 20) {
            Serial.println("corrige derecha");
            girarServo(2);            
          } else {
            girarServo(0);
          }
        }
      }
      if (vueltas == 0 && dfFrente<45) {
        currentState = SENTIDO_ABIERTA;
        Serial.println("SENTIDO_ABIERTA");
        break;
      }
      if (dfFrente < 45 && dEncoder > 170) {
        giraAbierta();  
      }
      break;

    case SENTIDO_ABIERTA:
      mostrarDistancias();
      if (dfDer > dfIzq){
        sentido = 1;
      }else{
        sentido = -1;
      }
      currentState = GIRAR_ABIERTA;
      giraAbierta();
      break;

    case GIRAR_ABIERTA:
      Serial.print("encoder ");
      Serial.println(dEncoder);
      if (true) {
        vueltas++;
        resetEncoder();
        Serial.println(vueltas);
        if (vueltas >= turns) {
          currentState = AVANCE_FINAL_ABIERTA;
          Serial.println("AVANCE_FINAL_ABIERTA");
        } else {
          currentState = AVANZAR_ABIERTA;
          Serial.println("AVANZA_ABIERTA");
        }
      }
      break;

    case AVANCE_FINAL_ABIERTA:
      controlMotor(1);
      girarServo(0);
      if (dEncoder > 50) {
        currentState = DETENERSE_ABIERTA;
        Serial.println("DETENERSE_ABIERTA");
      }
      break;

    case DETENERSE_ABIERTA:
      controlMotor(0);
      break;

    case AVANZAR_OBSTACULOS:
      controlMotor(1);
      if (vueltas == 0 && dfFrente<65) {
        currentState = SENTIDO_OBSTACULOS;
        Serial.println("SENTIDO_OBSTACULOS");
        break;
      }


      if (vueltas != 0) {
        if (sentido == -1) {
          if (dfDer > 50) {
            Serial.println("corrige derecha");
            girarServo(2);
            delay(100);
          }
          else if (dfDer < 35) {
            Serial.println("corrige izquierda");
            girarServo(-2);
            delay(100);
          } else {
            girarServo(0);
            delay(100);
          }
        } else {
          if (dfIzq > 50) {
            Serial.println("corrige izquierda");
            girarServo(-2);
          } else if (dfIzq < 35) {
            Serial.println("corrige derecha");
            girarServo(2);            
          } else {
            girarServo(0);
          }
        }
      }
      
      if (dfFrente < 65) {
        giraObstaculos();  
      }
      break;
  
    case SENTIDO_OBSTACULOS:
      mostrarDistancias();
      if (dfDer > dfIzq){
        sentido = 1;
      }else{
        sentido = -1;
      }
      currentState = GIRAR_OBSTACULOS;
      giraObstaculos();
      break;
    

    case GIRAR_OBSTACULOS:
      Serial.print("encoder ");
      Serial.println(dEncoder);
      vueltas++;
      resetEncoder();
      Serial.println(vueltas);
      if (vueltas >= turns) {
        currentState = AVANCE_FINAL_OBSTACULOS;
        Serial.println("AVANCE_FINAL_OBSTACULOS");
      } else {
        currentState = ESQUIVA_OBSTACULOS;
        Serial.println("ESQUIVA_OBSTACULOS");
        obstaculo = 'Y';
      }
      break;
    
    case ESQUIVA_OBSTACULOS:
      if (obstaculo=='Y'){
        desifrarMensaje();
      } else {
        controlMotor(1);
        if (obstaculo=='R'){
          girarServo(1);
          delay(1000);
          girarServo(-1);
          delay(1000);
          girarServo(0);

        }
        if (obstaculo=='V'){
          girarServo(-1);
          delay(1000);
          girarServo(1);
          delay(1000);
          girarServo(0);

        }
        if (obstaculo=='N'){
          currentState = AVANZAR_OBSTACULOS;
        }
      }
      break;

    case AVANCE_FINAL_OBSTACULOS:
      controlMotor(1);
      girarServo(0);
      if (dEncoder > 50) {
        currentState = DETENERSE_OBSTACULOS;
        Serial.println("DETENERSE_OBSTACULOS");
      }
      break;
      
    case DETENERSE_OBSTACULOS:
      controlMotor(0);
      break;
  }
}

void desifrarMensaje(){
  if (Serial.available()) {
    String mensaje = Serial.readStringUntil('\n'); 
    if (mensaje[0] == 'V') obstaculo = 'V';
    if(mensaje[0] == 'R') obstaculo = 'R';
  }
}

void giraObstaculos(){
  controlMotor(0);
  girarServo(sentido);
  delay(2000);
  controlMotor(1);
  delay(2100);
  resetEncoder();
  currentState = ESQUIVA_OBSTACULOS;
  Serial.println("ESQUIVAR_OBSTACULOS");
}

void giraAbierta(){
  currentState = GIRAR_ABIERTA;
  Serial.println("GIRAR_ABIERTA");
  controlMotor(0);
  girarServo(sentido);
  delay(2000);
  controlMotor(1);
  delay(2100);
  resetEncoder();
}

int anterior= 253235;
// ==================== SERVO ====================
void girarServo(int i) {
  if(i==anterior) return;
  anterior = i;
  Serial.print("S");   // Enviar a Raspberry
  Serial.println(i);
}

// ==================== MOTOR ====================
void setupMotor() {
  pinMode(PIN_IN1A, OUTPUT);
  pinMode(PIN_IN2A, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, HIGH);
}

void controlMotor(int i) {
  int velocidad = 0;
  switch (i) {
    case -1:
      analogWrite(PIN_PWMA, VELOCIDAD_NORMAL);
      digitalWrite(PIN_IN1A, LOW);
      digitalWrite(PIN_IN2A, HIGH);
      return;
    case 0:
      digitalWrite(PIN_IN1A, LOW);
      digitalWrite(PIN_IN2A, LOW);
      return;
    case 1: velocidad = 100; break;
    case 2: velocidad = 130; break;
    case 3: velocidad = 160; break;
    case 4: velocidad = 190; break;
    case 5: velocidad = 220; break;
    case 6: velocidad = 255; break;
    default: velocidad = VELOCIDAD_NORMAL; break;
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
void BotonVerdePresionado() {
  if (digitalRead(PIN_BOTON_VERDE) == LOW) {
    BV = true;
  }
}
void BotonAzulPresionado() {
  if (digitalRead(PIN_BOTON_AZUL) == LOW) {
    BA = true;
  }
}

// ==================== ULTRASONICOS ====================
void setupUltrasonicos() {
  pinMode(PIN_TRIG_FRENTE, OUTPUT);
  pinMode(PIN_ECHO_FRENTE, INPUT);
  pinMode(PIN_TRIG_DER, OUTPUT);
  pinMode(PIN_ECHO_DER, INPUT);
  pinMode(PIN_TRIG_IZQ, OUTPUT);
  pinMode(PIN_ECHO_IZQ, INPUT);
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

unsigned long lastUpdate = 0;
const unsigned long interval = 30; // cada 30 ms ≈ 33 Hz

void actualizarDistancias() {
  /*
  if (dF > 0) dfFrente = dF;
if (dD > 0) dfDer = dD;
if (dI > 0) dfIzq = dI;
  */
  if (millis() - lastUpdate < interval) return; // aún no toca actualizar
  lastUpdate = millis();
  
  // Lecturas en bruto
  float dF = leerUltrasonico(PIN_TRIG_FRENTE, PIN_ECHO_FRENTE);
  float dD = leerUltrasonico(PIN_TRIG_DER, PIN_ECHO_DER);
  float dI = leerUltrasonico(PIN_TRIG_IZQ, PIN_ECHO_IZQ);

  // Si la lectura es válida (no cero), aplica el filtro; si no, conserva el valor anterior
  if (dF > 0) dfFrente = alpha * dF + (1 - alpha) * dfFrente;
  if (dD > 0) dfDer = alpha * dD + (1 - alpha) * dfDer;
  if (dI > 0) dfIzq = alpha * dI + (1 - alpha) * dfIzq;

  // Encoder -> metros (ajusta divisor según tu resolución)
  dEncoder = (float)posEncoder / 39.0;
}


void mostrarDistancias() {
  Serial.print(dfFrente, 2); Serial.print("\t");
  Serial.print(dfDer, 2); Serial.print("\t");
  Serial.print(dfIzq, 2); Serial.print("\t");
  Serial.println(dEncoder);
}

// ==================== ENCODER ====================
void setupEncoder() {
  pinMode(PIN_ENCA, INPUT);
  pinMode(PIN_ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCA), readEncoder, RISING);
}

void readEncoder() {
  int b = digitalRead(PIN_ENCB);
  if (b > 0) posEncoder++;
  else posEncoder--;
}

void resetEncoder() {
  posEncoder = 0;
  dEncoder = 0;
}

void pruebaAbierta(){
  
}
