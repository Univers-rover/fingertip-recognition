#include <opencv2/opencv.hpp>
#include "face_dect.h"

using namespace cv;

void face_detection()
{
	Mat img = imread("./demo.jpg");
	Mat DstPic, edge, grayImage;

	DstPic.create(img.size(), img.type());
	cvtColor(img, grayImage, COLOR_BGR2GRAY);
	blur(grayImage, edge, Size(3, 3));

	//运行canny算子
	Canny(edge, edge, 3, 4, 3);
	imwrite("Canny.jpg", edge);

}

