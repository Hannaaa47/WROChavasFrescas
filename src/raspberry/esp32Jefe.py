import threading
import RPi.GPIO as GPIO
import time
import serial

# ---------- CONFIGURACION SERVO ----------
GPIO.setmode(GPIO.BCM)
GPIO.setup(3, GPIO.OUT)

servo = GPIO.PWM(3, 50)  # 50 Hz
servo.start(0)

# Posiciones (ajusta segn tu servo real)
derecha = 6.5 #7.5 6
der_medio = 5 	
der_a_centro = 4.2
izq_a_centro = 4.6
izq_medio = 3.5
izquierda = 2.5 #4.5c2

# Variables compartidas
ultima_orden = None
lock = threading.Lock()
anterior = 3242

def servo_worker():
    global ultima_orden, derecha, izq_a_centro, der_a_centro, izquierda, anterior 
    while True:
        with lock:
            orden = ultima_orden
        if orden is not None and orden != anterior:
            if orden == -2 :
                direccion = izq_medio
            elif orden == -1 :
                direccion = izquierda
            elif orden == 0 :
                if anterior == 1 :
                    direccion = der_a_centro
                elif anterior == -1 :
                    direccion = izq_a_centro
                elif anterior == -2:
                    direccion = 4.5
                else :
                    direccion = der_a_centro
            elif orden == 1 :
                direccion = derecha
            elif orden == 2 :
                direccion = der_medio
            else :
                direccion = None

            if direccion is not None:
                print("Servo movindose a:", direccion)
                servo.ChangeDutyCycle(direccion)
                time.sleep(0.5)   # Espera para que llegue
                servo.ChangeDutyCycle(0)  # Evita temblores
                anterior = orden


def leer_serial():
    global ultima_orden
    esp32 = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    esp32.reset_input_buffer()
    while True:
        if esp32.in_waiting > 0:
            line = esp32.readline().decode("utf-8", errors="ignore").rstrip()
            
            with lock:
                if line == "S2":
                    ultima_orden = 2
                elif line == "S1":
                    ultima_orden = 1
                elif line == "S-1":
                    ultima_orden = -1
                elif line == "S-2":
                    ultima_orden = -2
                elif line == "S0":
                    ultima_orden = 0
                else:
                    print("Recibido:", line)

# Lanzar hilos
t1 = threading.Thread(target=leer_serial, daemon=True)
t2 = threading.Thread(target=servo_worker, daemon=True)

t1.start()
t2.start()

# Mantener vivo el programa principal
while True:
    time.sleep(1)
