CC=g++

all: intersection car_client car_listener testcv objdettut

intersection: intersection.cpp
	$(CC) -std=c++11 -fopenmp -o intersection intersection.cpp

car_client: car_client.cpp
	$(CC) -std=c++11 -fopenmp -o car_client car_client.cpp

car_listener: car_listener.cpp
	$(CC) -std=c++11 -fopenmp -o car_listener car_listener.cpp

testcv:testcv.cpp
	$(CC) -std=c++11 testcv.cpp -I /usr/include/opencv2 -L /usr/lib -lopencv_core -opencv_imgproc -lopencv_highgui -lopencv_video -o testcv 

objdettut:objdettut.cpp
	$(CC) -std=c++11 objdettut.cpp -I /usr/include -L /usr/lib -lm -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -o objdettut
