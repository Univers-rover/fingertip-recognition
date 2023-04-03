#include<iostream>
 
#include<opencv2/core/core.hpp>
#include<opencv2/opencv.hpp>
 
using namespace std;
using namespace cv;
//this program is used for testing opencv encode and decode for jgeg pictures
int main()
{
    Mat tstMat=imread("/home/zhy/Documents/Perception/camera_data/lane_image/data/image/1/0000.jpg");
   // imshow("picture",tstMat);
    vector<unsigned char> inImage;
    imencode(".jpg",tstMat,inImage);
    size_t datalen=inImage.size();
    
    unsigned char *msgImage = new unsigned char[datalen];
    
    for(int i=0;i<datalen;i++)
    {
        msgImage[i]=inImage[i];
        //cout<<msgImage[i]<<endl;
    }
 
    vector<unsigned char> buff;
    for(int i=0;i<datalen;i++)
    {
           buff.push_back(msgImage[i]);
    }
    Mat show=imdecode(buff, IMREAD_UNCHANGED);
    imshow("picture",show);
 
    cv::waitKey(0);
    cout<<"hello world"<<endl;
    return 0;
}
