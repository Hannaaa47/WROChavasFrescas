# Python code for Multiple Color Detection 

from picamera2 import Picamera2
import cv2;
import numpy as np;  
import time;

#picam2 = Picamera2();
#picam2.configure(picam2.create_preview_configuration(
 #   main={"format": 'BGR888', "size": (640, 480)}
#));
#picam2.start();

#picam2.image_effect = 'none';

#Rango de colores en HSV
verde_bajo = np.array([40, 70, 70]);
verde_alto = np.array([80, 255, 255]);

#rojo_bajo1 = np.array([0, 100, 100]);
#rojo_alto1 = np.array([10, 255, 255]);

rojo_bajo2 = np.array([160, 100, 100]);
rojo_alto2 = np.array([179, 255, 255]);

#iniciar camara
#cam = cv2.VideoCapture(0, cv2.CAP_V4L2);  # Para V4L2 en Linux
picam2 = Picamera2();
picam2.preview_configuration.main.size = (640, 480);
picam2.preview_configuration.main.format = "BGR888";
picam2.configure("preview");
picam2.start();

time.sleep(1); #Dejar tiempo para que se estabilice la camara

while True:
    frame = picam2.capture_array();
    frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR);
        
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV);
    
    mascara_verde = cv2.inRange(hsv, verde_bajo, verde_alto);
    #mascara_rojo1 = cv2.inRange(hsv, rojo_bajo1, rojo_alto1);
    #mascara_rojo2 = cv2.inRange(hsv, rojo_bajo2, rojo_alto2);
    mascara_rojo  = cv2.inRange(hsv, rojo_bajo2, rojo_alto2);
    
    pixeles_verde = cv2.countNonZero(mascara_verde);
    pixeles_rojo  = cv2.countNonZero(mascara_rojo);
    
    if pixeles_verde > 5000:
        print("verde detectado, girar a la izq");
    elif pixeles_rojo > 5000:
        print("rojo detectado, girar a la derecha");
    else:
        print("sin color detectado");
    
    cv2.imshow("Camara", frame);
    
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break;
    
cv2.destroyAllWindows();
picam2.close();
    
