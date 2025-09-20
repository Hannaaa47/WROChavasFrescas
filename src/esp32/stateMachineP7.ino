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
int dfChoque = 40, correInt = 40, correExt = 20 ; 

// -------------- Variables state machine
enum State { INICIO, AVANZAR, DECIDIR_SENTIDO, GIRAR, AVANCEFINAL, DETENERSE };
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
  //mostrarDistancias();
  stateMachine();
  actualizarDistancias();
}

void stateMachine() {
  switch (currentState) {
    case INICIO:
      BotonVerdePresionado();
      BotonAzulPresionado();
      if (BV == true) {
        girarServo(1);
        delay(2000);
        girarServo(0);
        delay(2000);
        currentState = AVANZAR;
        Serial.println("AVANZA");
        resetEncoder();
      }
      break;

    case AVANZAR:
      
      controlMotor(1);

      if (vueltas != 0) {
        if (sentido == -1) {
          if (dfDer > correInt && dfDer < correExt){
            girarServo(0);
          } else if (dfDer > correInt) { //40
            Serial.print("corrige derecha ");
            Serial.println(dfDer);
            girarServo(2);
            delay(50);
          } else if (dfDer < correExt) { //20
            Serial.print("corrige izquierda ");
            Serial.println(dfDer);
            girarServo(-2);
            delay(50);
          } 
        } else {
          if (dfIzq > correInt && dfIzq < correExt){
            girarServo(0);
          } else if (dfIzq > correInt) {
            Serial.print("corrige izquierda ");
            Serial.println(dfIzq);
            girarServo(-2);
          } else if (dfIzq < correExt) {
            Serial.print("corrige derecha ");
            Serial.println(dfIzq);
            girarServo(2);            
          }
        }
      }
      if (vueltas == 0 && dfFrente<dfChoque) {
        currentState = DECIDIR_SENTIDO;
        Serial.println("DECIDIR SENTIDO");
        break;
      }
      if (dfFrente < dfChoque && dEncoder > 170) {
        gira(); 
      }
      break;

    case DECIDIR_SENTIDO:
      mostrarDistancias();
      if (dfDer > dfIzq){
        sentido = 1;
      }else{
        sentido = -1;
      }
      gira();
      break;

    case GIRAR:
      Serial.print("encoder ");
      Serial.println(dEncoder);
      if (true) {
        vueltas++;
        resetEncoder();
        Serial.println(vueltas);
        if (vueltas >= turns) {
          currentState = AVANCEFINAL;
          Serial.println("AVANCE FINAL");
        } else {
          currentState = AVANZAR;
          Serial.println("AVANZA");
        }
      }
      break;

    case AVANCEFINAL:
      controlMotor(1);
      girarServo(0);
      if (dEncoder > 50) {
        currentState = DETENERSE;
        Serial.println("DETENERSE");
      }
      break;

    case DETENERSE:
      controlMotor(0);
      break;
  }
}

void gira(){
  currentState = GIRAR;
  Serial.println("GIRAR");
  controlMotor(0);      //se detiena
  girarServo(sentido);  //gira para dar la vuelta
  delay(1000);          // espera para que el raspberry responda
  controlMotor(1);      //avanza
  delay(2000);          // este es el tiempo que va a girar
  Serial.println("deja de girar");
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
