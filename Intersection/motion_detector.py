import argparse
import datetime
import imutils
import time
import cv2
from client import stopClient 

global x, y, w, h

def getPoints():
	return (x,y,w,h)

ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video", help="path to the video file")
ap.add_argument("-a", "--min-area", type=int, default=500, help="minimum area size")
args = vars(ap.parse_args())

if args.get("video", None) is None:
	camera = cv2.VideoCapture(1)
	#camera.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH,640.0)
	#camera.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT,480.0)
	time.sleep(0.25)
else:
	camera = cv2.VideoCapture(args["video"])

firstFrame = None
while True:
	(grabbed, frame) = camera.read()
	text = "Nothing Present"
	if not grabbed:
		break

	frame = imutils.resize(frame, width=700)
	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	gray = cv2.GaussianBlur(gray, (21, 21), 0)

	if firstFrame is None:
		firstFrame = gray
		continue

	frameDelta = cv2.absdiff(firstFrame, gray)
	thresh = cv2.threshold(frameDelta, 40, 255, cv2.THRESH_BINARY)[1]
	thresh = cv2.dilate(thresh, None, iterations=1)
	(cnts, _) = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)

	for c in cnts:
		# if the contour is too small, ignore it
		if cv2.contourArea(c) < args["min_area"]:
			continue

		(x, y, w, h) = cv2.boundingRect(c)
		
		stopClient(x,y,w,h,)
		cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 0, 255), 2)
		
		print getPoints()
		text = "Occupied"

	cv2.putText(frame, "Intersection Status: {}".format(text), (10, 20),
		cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)

	# show the frame and record if the user presses a key
	cv2.line(frame, (40,290),(340,450),(0,255,0),2)
	cv2.line(frame, (340,450),(680,240),(0,255,0),2)
	cv2.line(frame, (680,240),(357,66),(0,255,0),2)
	cv2.line(frame, (357,66),(40,290),(0,255,0),2)
	cv2.imshow("Intersection Feed", frame)
	cv2.imshow("Thresh", thresh)
	#cv2.imshow("Frame Delta", frameDelta)
	key = cv2.waitKey(1) & 0xFF

	# if the `q` key is pressed, break from the lop
	if key == ord("q"):
		break

# cleanup the camera and close any open windows
camera.release()
cv2.destroyAllWindows()
