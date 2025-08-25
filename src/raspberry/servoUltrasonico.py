from picamera2 import Picamera2
import serial
import cv2
import numpy as np
import RPi.GPIO as GPIO
import time


# === Configuracion de camara ===
picam2 = Picamera2()
picam2.preview_configuration.main.size = (640, 480)
picam2.preview_configuration.main.format = "BGR888"
picam2.configure("preview")
picam2.start()

#configuracion del servo
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(3, GPIO.OUT)

p = GPIO.PWM(3, 50)  # 35 Hz
p.start(7.0)          # Centro

def giraDer() :
	p.ChangeDutyCycle(12.5) # Derecha
	time.sleep(2)
	p.ChangeDutyCycle(0)  
	
def giraIzq() :
	p.ChangeDutyCycle(2.5) # Izquierda
	time.sleep(2)
	p.ChangeDutyCycle(0)  
	
def giraCen() :
	p.ChangeDutyCycle(7.0) # Centro
	time.sleep(2)
	p.ChangeDutyCycle(0)  

# === Funcion para deteccion de lado libre ===
def detectar_lado_libre(frame):
    brillo = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    mitad = frame.shape[1] // 2
    izquierda = brillo[:, :mitad]
    derecha = brillo[:, mitad:]

    avg_izq = np.mean(izquierda)
    avg_der = np.mean(derecha)

    if avg_izq > avg_der + 10:
        return "IZQUIERDA"
    elif avg_der > avg_izq + 10:
        return "DERECHA"
    else:
        return None
        

if __name__ == '__main__':
	esp32 = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
	esp32.reset_input_buffer()
	esp32.write(b"B1\n")
	line=""
	while(True):
		if esp32.in_waiting > 0:
			line = esp32.readline().decode("utf-8").rstrip()
			print(f"Recibido del ESP32: {line}")
            #print(type(line)) 
			if (line=="Boton verde presionado"):
				break;
	esp32.write(b"B0\n")
	if (line=="Boton verde presionado"):   #Inicia la prueba abierta
		#revisar sentido
		try:
			while True:
				frame = picam2.capture_array()
				frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

				direccion = detectar_lado_libre(frame)

				if direccion == "IZQUIERDA":
					print("Obstaculo a la derecha, girando servo a la izquierda")
					#servo.ChangeDutyCycle(2.9)  # Izquierda
					break
				elif direccion == "DERECHA":
					print("Obstaculo a la izquierda, girando servo a la derecha")
					#servo.ChangeDutyCycle(12.9) # Derecha
					break

				cv2.imshow("Camara", frame)
				if cv2.waitKey(1) & 0xFF == ord('q'):
					break

		except KeyboardInterrupt:
			pass
		
		esp32.write(b"M1\n")
		for i in range (0,12):
			esp32.write(b"U1\n")
			distancia= 220304203.0
			esp32.reset_input_buffer()
			while(distancia>7):
				#print("Comando enviado a ESP32")
				if esp32.in_waiting > 0:
					line = esp32.readline().decode("utf-8").rstrip()
					print(f"Recibido del ESP32: {line}")
					#print(type(line)) 
					try:
						distancia = float(line)
						if distancia >= 0 and distancia <= 1:  # fuera de rango tpico
							continue
						#print("Distancia valida:", distancia)
					except ValueError:
						continue

			esp32.write(b"U0\n")
			giraDer()
			time.sleep(4)
			giraCen()
		
	esp32.write(b"M0\n")
    
