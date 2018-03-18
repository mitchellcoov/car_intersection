import numpy as np
import cv2, urllib, time
from client import stopClient 

stop_cascade = cv2.CascadeClassifier('haarcascade_stopsign.xml')

cap = cv2.VideoCapture(1)

cv2.namedWindow('Feed', cv2.WINDOW_NORMAL)
cv2.resizeWindow('Feed', 1280, 700)

while True:
    ret, img = cap.read()

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    stopSign = stop_cascade.detectMultiScale(gray, 1.3, 5)

    for (sx,sy,sw,sh) in stopSign:
        cv2.rectangle(img,(sx,sy),(sx+sw,sy+sh),(0,0,255),2)
	print sw*sh
	stopClient(sw*sh)

    cv2.putText(img, "Red=Stop", (0, 30), cv2.FONT_ITALIC, 1, (0, 0, 255))

    cv2.imshow('Feed',img)

    k = cv2.waitKey(1)
    if k == 27:
        break

cap.release()
cv2.destroyAllWindows()
