//Prueba 1 para hacer el open challenge

//PINES
const int PIN_BOTON_VERDE = 12; //boton verde : para la prueba abierta
const int PIN_BOTON_AZUL = 13; //boton azul : para la prueba de obstaculos
const int PIN_PWMA = 25; //motor : PWM (velocidad)
const int PIN_IN1A = 27; //motor : Dirección
const int PIN_IN2A = 26; //motor : Dirección
const int PIN_STBY = 14; //motor : Standby (HIGH = activo)

//VARIABLES
bool estadoAnteriorBoton_v = HIGH;
bool estadoAnteriorBoton_a = HIGH;
bool botonPresionado_v = false;
bool botonPresionado_a = false;
int velocidad= 100;

void setup() {
  Serial.begin(9600);

  //
  pinMode(PIN_BOTON_VERDE, INPUT_PULLUP); //boton verde
  pinMode(PIN_BOTON_AZUL, INPUT_PULLUP); //boton azul
  pinMode(PIN_IN1A, OUTPUT); //motor
  pinMode(PIN_IN2A, OUTPUT); //motor
  pinMode(PIN_PWMA, OUTPUT); //motor
  pinMode(PIN_STBY, OUTPUT); //motor

  digitalWrite(PIN_STBY, HIGH);  // Desactivar standby
  detener();
}

void loop() {
  //Crea la comunicacion con el raspberry pi
  /*
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    Serial.print("You sent me: ");
    Serial.println(data);
  }
  */

  detectarBotonVerde();
  detectarBotonAzul();

  procesarComandosSerial();
  
  }

void procesarComandosSerial() {
  if (Serial.available() > 0) {
    char comando = Serial.read(); // Leer un solo carácter como comando

    switch (comando) {
      case 'F': // Motor hacia adelante
        adelante();
        break;
      case 'R': // Motor gira en reversa
        reversa();
        break;
      case 'D': // Motor se detiene
        detener();
        break;
      case 'V': // Estado botón verde (quizás para RPi consultar el estado)
        Serial.println((botonPresionado_v ? 'Y' : 'N')); // RPi preguntaría, ESP32 responde
        break;
      case 'A': // Estado botón azul
        Serial.println((botonPresionado_a ? 'Y' : 'N'));
        break;
      // Puedes añadir más comandos si es necesario (ej. para cambiar velocidad)
      default:
        Serial.print("Comando desconocido: ");
        Serial.println(comando);
        break;
    }
  }

  //Define un protocolo de comunicación sencillo. 
  //Recibidos -> De raspberry a esp32
  // V : estado boton verde
  // A : estado boton azul
  // F : motor hacia adelante
  // R : motor gira en reversa
  // D : motor se detiene
  //Enviados
  // OpenChallenge : Inicio open challenge
  // ObstacleChallenge : Inicio obstacle challenge
  // OK : ???

}


void estadoBotonVerde(){
  bool estadoBoton_v = digitalRead(PIN_BOTON_VERDE);

  if (estadoBoton_v == LOW && estadoAnteriorBoton_v == HIGH) { // Detectar flanco de bajada (presión del botón)
    Serial.println("OpenChallenge"); //Boton verde presionado
    botonPresionado_v= true;
    delay(50); // Pequeño delay para evitar rebotes
  }

  estadoAnteriorBoton_v = estadoBoton_v;
}

void estadoBotonAzul(){
  bool estadoBoton_a = digitalRead(PIN_BOTON_AZUL);

  if (estadoBoton_a == LOW && estadoAnteriorBoton_a == HIGH) { // Detectar flanco de bajada (presión del botón)
    Serial.println("ObstacleChallenge"); //Boton azul presionado
    botonPresionado_a= true;
    delay(50); // Pequeño delay para evitar rebotes
  }

  estadoAnteriorBoton_a = estadoBoton_a;
}

void adelante(){ //forward
  digitalWrite(PIN_STBY, HIGH);
  digitalWrite(PIN_IN1A, HIGH); 
  digitalWrite(PIN_IN2A, LOW);
  analogWrite(PIN_PWMA, velocidad);
  //Serial.println("Motor: Adelante"); // Para depuración
} 

void reversa(){
  digitalWrite(PIN_STBY, HIGH);
  digitalWrite(PIN_IN1A, LOW); 
  digitalWrite(PIN_IN2A, HIGH);
  analogWrite(PIN_PWMA, velocidad);
  //Serial.println("Motor: Reversa"); // Para depuración
} 

void detener(){
  digitalWrite(PIN_IN1A, LOW); 
  digitalWrite(PIN_IN2A, LOW);
  analogWrite(PIN_PWMA, 0);
  digitalWrite(PIN_STBY, LOW);
  //Serial.println("Motor: Detenido"); // Para depuración
}
