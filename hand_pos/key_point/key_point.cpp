#include "key_point.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;

static string protoFile = "pose_deploy.prototxt";
static string model_weight = "pose_iter_102000.caffemodel";//网络训练权重

//加载caffe模型
static Net g_net = readNetFromCaffe(protoFile, model_weight);

//手部关键点检测
bool HandKeypoints_Detect(Mat src, vector<Point>&HandKeypoints)
{
	//Mat src = imread(imageList[i]);//读取

	//模型尺寸大小
	int width = src.cols;
	int height = src.rows;
	float ratio = width / (float)height;
	int modelHeight = 368;  //由模型输入维度决定
	int modelWidth = int(ratio*modelHeight);

	//将输入图像转成blob形式
	Mat blob = blobFromImage(src, 1.0 / 255, Size(modelWidth, modelHeight), Scalar(0, 0, 0));

	//将图像转换的blob数据输入到网络的第一层“image”层，见deploy.protxt文件
	g_net.setInput(blob, "image");

	//结果输出
	Mat output = g_net.forward();
	int H = output.size[2];
	int W = output.size[3];

	/*
	for (int i = 0; i < nPoints; i++)
	{
		//结果预测
		Mat probMap(H, W, CV_32F, output.ptr(0, i)); 

		resize(probMap, probMap, Size(width, height));

		Point keypoint; //最大可能性手部关键点位置，Ppint下有Point.x和Point.y
		double classProb;  //最大可能性概率值
		minMaxLoc(probMap, NULL, &classProb, NULL, &keypoint);//opencv函数，查找最大概率的位置

		HandKeypoints[i] = keypoint; //结果输出，即手部关键点所在坐标
	}
	*/

	Mat probMap(H, W, CV_32F, output.ptr(0, 7));

	resize(probMap, probMap, Size(width, height));

	Point keypoint;
	double classProb;  //最大可能性概率值
	minMaxLoc(probMap, NULL, &classProb, NULL, &keypoint);//opencv函数，查找最大概率的位置
	
	HandKeypoints[0] = keypoint;

	return true;
}

void run(void) {
    int i = 0;
    vector<string>imageList;
    string filename = "./photos/";
    //vector<Point>HandKeypoints(nPoints);
	vector<Point>HandKeypoint(2);

	glob(filename, imageList);

    for (i = 0; i < imageList.size(); i++)
    {
        Mat src = imread(imageList[i]);//读取

        HandKeypoints_Detect(src, HandKeypoint);

        //printf("image%d:x=%d,y=%d \n", i, HandKeypoints(7).x, HandKeypoints(7).y);
		cout<<"image"<<i<<":x="<<HandKeypoint[0].x<<"y="<<HandKeypoint[0].y<<endl;
    }
}
