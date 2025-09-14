import serial

import numpy as np
import RPi.GPIO as GPIO
import time

# Configuracion del servo
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(3, GPIO.OUT)

p = GPIO.PWM(3, 50)  # 50 Hz
p.start(7.0)         # Centro

# Distancia minima ultrasonico
distanciaMin = 40

def giraDer():
    p.ChangeDutyCycle(9.5)  # Derecha
    time.sleep(2)
    p.ChangeDutyCycle(0)

def giraIzq():
    p.ChangeDutyCycle(2.5)  # Izquierda
    time.sleep(2)
    p.ChangeDutyCycle(0)

def giraCen():
    p.ChangeDutyCycle(6.5)  # Centro
    time.sleep(2)
    p.ChangeDutyCycle(0)

# === Funcion para deteccion de lado libre ===
def detectar_lado_libre():
    # Aqui deberias implementar la deteccion real con OpenCV o sensores
    return "DERECHA"   # De momento fijo para pruebas


if __name__ == '__main__':
    esp32 = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    esp32.reset_input_buffer()
    line = ""
    esp32.write(b"B1\n")
    giraCen();
    # Espera a que se presione el boton verde en el ESP32
    while True:
        if esp32.in_waiting > 0:
            line = esp32.readline().decode("utf-8").rstrip()
            if line == "Boton verde presionado":
                break

    # Enviar comando de inicio
    esp32.write(b"B0\n")

    if line == "Boton verde presionado":   # Inicia la prueba abierta
        try:
            while True:
                direccion = detectar_lado_libre()
                # Aqui podrias hacer algo con la direccion detectada
                # por ahora solo sigue al bloque de control de distancia
                break
				
            esp32.write(b"M1\n")
            for i in range(1):
                esp32.write(b"U1\n")
                distancia = 9999.0
                esp32.reset_input_buffer()

                # Espera hasta recibir una distancia valida
                while distancia > distanciaMin:
                    if esp32.in_waiting > 0:
                        line = esp32.readline().decode("utf-8").rstrip()
                        print(line)
                        try:
                            distancia = float(line)
                            if distancia <= 0 or distancia > 500:  # filtro de valores no realistas
                                continue
                        except ValueError:
                            continue

				# Decision segun lado libre
                if distancia < distanciaMin and detectar_lado_libre() == "IZQUIERDA":
                    giraIzq()
                elif distancia < distanciaMin and detectar_lado_libre() == "DERECHA":
                    giraDer()
                esp32.write(b"U0\n")
                time.sleep(1)
                giraCen()

            time.sleep(2)

        except KeyboardInterrupt:
            pass

        esp32.write(b"M0\n")
