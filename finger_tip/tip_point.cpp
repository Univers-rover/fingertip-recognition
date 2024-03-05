#include <iostream>
#include <cmath>
#include <numeric>
#include <vector>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/types_c.h>
//#include <opencv2/imgcodecs.hpp>
//#include "opencv2/imgcodecs/legacy/constants_c.h"
  
#include "tip_point.h"

using namespace std;
using namespace cv;

Mat img;

/* 肤色分割,将非肤色区域像素全置为0 */
Mat YCrCb_Otsu_detect(Mat & src){
	Mat ycrcb_image, detect;
	cvtColor(src, ycrcb_image, CV_BGR2YCrCb); //首先将RGB转换成到YCrCb空间
	
	vector<Mat> channels;
	split(ycrcb_image, channels);  //通道分离
	Mat output_mask = channels[1];  //Cr分量
	threshold(output_mask, output_mask, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);   //OSTU阈值分割
	src.copyTo(detect, output_mask);
	return detect;
}

/* 计算轮廓并求重心 */
void my_contour(Mat *src, vector<Point> *contours , Point *cen){
    Moments M;    //声明一个图像的矩
    vector<vector<Point>> all_contours;
    vector<Point> out_contours;
    vector<Vec4i> hierarchy;

    //计算轮廓
    findContours(*src,all_contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point());
    *contours = all_contours[0];//保留最大的轮廓

    //计算要绘制轮廓的矩
    M = moments(*contours);
    //求取轮廓重心的X坐标
    double cX = double(M.m10 / M.m00);
    //求取轮廓重心的Y坐标
    double cY = double(M.m01 / M.m00);
    
    cen->x = (int)cX;              
    cen->y = (int)cY;

    /*
    Mat dst1 = *src.clone();
    for(int i=0;i<all_contours.size();++i){
    	//绘制轮廓
        drawContours(dst1,all_contours,i,Scalar(0,0,255),1,8);
		//获取轮廓面积和弧长
        cout<<"Area "<<i<<" = "<<contourArea(all_contours[i])<<",Length "<<i<<" = "<<arcLength(all_contours[i],true)<<endl;
    }
    //cout<<"x:"<<cX<<"y:"<<cY<<endl;//打印重心位置
    //circle(dst1, cen, 3, Scalar(0, 255, 120), -1);//在重心画圆
    */
}

/* 两点距离计算 */
int cout_distance(Point &p1, Point &p2){   
    return (int)sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)); 
}

/* 指尖识别 */
static int find_tip(Mat &src, Point *tip_points){
    unsigned int i;
    int tip_ind,mean_dis,max_dis=-999;

    Mat img = src;
	Mat grayImage;//灰度图像】
    Mat binImage;//二值图像
    vector<int> distances;

    vector<Point> contours;//轮廓点
    Point center;//重心
    Point temp;
    //肤色分割并二值化
    grayImage = YCrCb_Otsu_detect(img);
    cvtColor(grayImage, grayImage, COLOR_BGR2GRAY);
    threshold(grayImage, binImage, 2, 255, THRESH_BINARY);

    //计算轮廓和重心
    my_contour(&binImage, &contours, &center);
    //cout<<"x="<<center.x<<"y="<<center.y<<endl;

    for(i = 0; i<contours.size(); i++){
        distances.push_back(cout_distance(contours[i], center));
        if(distances[i] > max_dis){
            max_dis = distances[i];
            tip_ind = i;
        }
    }

    //计算轮廓到重心的平均距离
    mean_dis = (int)(accumulate(distances.begin(), distances.end(), 0) / contours.size());
    //候选点到重心的距离大于边缘到重心平均距离的1.6倍，即为指尖；否则不是指尖，也就是说没有手指伸出
    if( distances[tip_ind] > (mean_dis*1.6) ){
        *tip_points=contours[tip_ind];
        return 0;
    }
    else
        return -1;
}

//将系统时间转为字符串
static string GetLocalTimeWithMs(void){
    string defaultTime = "19700101000000000";
    try
    {
        struct timeval curTime;
        gettimeofday(&curTime, NULL);
        int milli = curTime.tv_usec / 1000;

        char buffer[80] = {0};
        struct tm nowTime;
        localtime_r(&curTime.tv_sec, &nowTime);//把得到的值存入临时分配的内存中，线程安全
        strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &nowTime);

        char currentTime[84] = {0};
        snprintf(currentTime, sizeof(currentTime), "%s%03d", buffer, milli);

        return currentTime;
    }
    catch(const std::exception& e)
    {
        return defaultTime;
    }
    catch (...)
    {
        return defaultTime;
    }
}

//************************************
// Method:    TransBufferToMat
// FullName:  图片buffer数据转换成 Mat数据格式;
// Access:    public 
// Returns:   cv::Mat
// Qualifier:
// Parameter: unsigned char * pBuffer  图片数据内容
// Parameter: int nWidth	图片的宽度
// Parameter: int nHeight  图片的高度
// Parameter: int nBandNum 每个像素包含数据个数 (1, 3, 4 ) 1:Gray 3:RGB 4:RGBA
// Parameter: int nBPB  每个像素数据 所占位数(1, 2) 1:8位  2:16位;
//************************************
Mat TransBufferToMat(unsigned char* pBuffer, int nWidth, int nHeight, int nBandNum, int nBPB){
	Mat mDst;
    /*
	if (nBandNum == 4)
	{
		if (nBPB == 1)
		{
			mDst = cv::Mat::zeros(cv::Size(nWidth, nHeight), CV_8UC4);
		}
		else if (nBPB == 2)
		{
			mDst = cv::Mat::zeros(cv::Size(nWidth, nHeight), CV_16UC4);
		}
	}
    */
	if (nBandNum == 3)
	{
		if (nBPB == 1)
		{
			mDst = cv::Mat::zeros(cv::Size(nWidth, nHeight), CV_8UC3);
		}
		else if (nBPB == 2)
		{
			mDst = cv::Mat::zeros(cv::Size(nWidth, nHeight), CV_16UC3);
		}
	}
	else if (nBandNum == 1)
	{
		if (nBPB == 1)
		{
			mDst = cv::Mat::zeros(cv::Size(nWidth, nHeight), CV_8UC1);
		}
		else if (nBPB == 2)
		{
			mDst = cv::Mat::zeros(cv::Size(nWidth, nHeight), CV_16UC1);
		}
	}
    printf("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	for (int j = 0; j < nHeight; ++j)
	{
		unsigned char* data = mDst.ptr<unsigned char>(j);
		unsigned char* pSubBuffer = pBuffer + (int)((nHeight-1-j)*nWidth*nBandNum*nBPB);
		//printf("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
        cout<<"nheight="<<nHeight<<endl;
        int aa = (int)((nHeight-1-j)*nWidth*nBandNum*nBPB);
        int bb = (int)(nWidth*nBandNum*nBPB);
        cout<<"len1 "<<aa<<endl;
        cout<<"len2 "<<bb<<endl;
        memcpy((void *)data, (void *)pSubBuffer, nWidth*nBandNum*nBPB);
	}
    //printf("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	if (nBandNum == 1)
	{
		cv::cvtColor(mDst, mDst, COLOR_GRAY2BGR);
	}
	else if (nBandNum == 3)
	{
		cv::cvtColor(mDst, mDst, COLOR_RGB2BGR);
	}
	else if (nBandNum == 4)
	{
		cv::cvtColor(mDst, mDst, COLOR_RGBA2BGR);
	}
    printf("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return mDst;
}

int tip_posi(unsigned char *fb, int len, int *x, int *y){
    int i, err;
    Point p;

    _InputArray pic_arr(fb, len);

    img = imdecode(pic_arr, IMREAD_UNCHANGED);//CV_LOAD_IMAGE_COLOR
    //imwrite("1.jpg", img);
    
    //申请临时buffer空间
    unsigned char* tmp_buffer = new unsigned char[img.rows * img.cols];
    tmp_buffer = pic_arr.data;
    memcpy(buffer, tmp_buffer, img.rows * img.cols);
    delete[] tmp_buffer;//释放buffer空间

    printf("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    err = find_tip(img, &p);
    circle(img, p, 4, Scalar(0, 255, 120), -1);//在重心画圆

    cvShowImage("tip", img);

    if (!err){
        *x = p.x;
        *y = p.y;
    }

    return err;
}


