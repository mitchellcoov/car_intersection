#include "highgui/highgui.hpp"
#include "imgproc/imgproc.hpp"
#include "video/background_segm.hpp"
#include "video/tracking.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, const char** argv)
{
 //
 //  Load the image from file
 //
 Mat LoadedImage;
 // Just loaded image Lenna.png from project dir to LoadedImage Mat
 LoadedImage = imread("WIN_20180224_18_24_42_Pro.jpg", 1);

////trap test/////

vector<Point> contour;
	contour.push_back(Point(275,460));
	contour.push_back(Point(1030,150));
	contour.push_back(Point(1585,600));
	contour.push_back(Point(870,925));

	// create a pointer to the data as an array of points (via a conversion to 
	// a Mat() object)

	const cv::Point *pts = (const cv::Point*) Mat(contour).data;
	int npts = Mat(contour).rows;
	
	// draw the polygon 

	polylines(LoadedImage, &pts,&npts, 1,true,Scalar(0,255,0),1,CV_8UC1, 0);
////trap test/////

 imwrite("Step1.JPG", LoadedImage);


 // Show what rectangle
 namedWindow("Step 2 draw Rectangle", WINDOW_AUTOSIZE);
 imshow("Step 2 draw Rectangle", LoadedImage);
 waitKey(5000);


}
