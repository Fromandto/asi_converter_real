#include "stdio.h"
#include "highgui.h"
#include "ASICamera.h"
#include <sys/time.h>
#include <time.h>
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sensor_msgs/fill_image.h>

/*unsigned long GetTickCount()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC , &ts);
	return (ts.tv_sec*1000 + ts.tv_nsec/(1000*1000));
}*/

int main(int argc , char ** argv)
{
	ros::init(argc , argv , "asi_converter");
	ros::NodeHandle nh_;
	image_transport::ImageTransport it_(nh_);
	image_transport::Publisher image_pub_ = it_.advertise("/usb_cam/image_raw" , 1);
	ros::Rate loop_rate(80);
	//cv::VideoCapture capture;
	//capture.open("/root/catkin_ws/test.avi");
	//assert(capture.isOpened());
	//cv::namedWindow("Preview");
	
	//declare for asi api
	int width;
	char * bayer[] = {"RG" , "BG" , "GR" , "GB"};
	char * controls[7] = {"Exposure" , "Gain" , "Gamma" , "WB_R" , "WB_B" , "Brightness" , "USB Traffic"};
	int height;
	int i;
	char c;
	bool bresult;
	int time1 , time2;
	int count = 0;
	char buf[128] = {0};
	int CamNum = 0;
	IplImage *pRgb;
	int numDevices = getNumberOfConnectedCameras();
	if(numDevices <= 0)
	{
		printf("no camera connected , press any key to exit\n");
		getchar();
		return -1;
	}
	CamNum = 0;
	bresult = openCamera(CamNum);
	if(!bresult)
	{
		printf("openCamera error , are you root ? press any key to exit \n");
		getchar();
		return -1;
	}
	width = 752;
	height = 480;
	initCamera();
	printf("sensor temprature: %02f\n" , getSensorTemp());
	while(!setImageFormat(width , height , 1 , IMG_RAW8))
	{
		printf("please input right resolution:\n");
		scanf("%d %d" , &width , &height);
	}
	printf("\n set image format success , press ESC to stop\n");
	cvNamedWindow("video" , 1);
	pRgb = cvCreateImage(cvSize(getWidth() , getHeight()) , IPL_DEPTH_8U , 1);
	setValue(CONTROL_EXPOSURE , 10 * 1000 , false); // auto exposure
	setValue(CONTROL_GAIN , 50 , false); //auto gain
	setValue(CONTROL_BANDWIDTHOVERLOAD , getMin(CONTROL_BANDWIDTHOVERLOAD) , false);
	setValue(CONTROL_WB_B , 90 , false);
	setValue(CONTROL_WB_R , 48 , false);
	setAutoPara(50 , 10 , 150);
	startCapture();
	//time1 = GetTickCount();
	int sign = 0;

	while(ros::ok())
	{
		getImageData((unsigned char*)pRgb->imageData , pRgb->imageSize , -1);
		count ++;
		//time2 = GetTickCount();
		sign ++;
		//if(sign < 3)
		//	continue;
		//sign = 0;
		char c = cvWaitKey(1);
		switch(c)
		{
			case 27 :
				goto END;
		}
		cvShowImage("video" , pRgb);
		//capture >> frame;
		//cv::imshow("Preview" , frame);
		//cv::waitKey(1);
		cv::Mat frame(pRgb , 0);
		std_msgs::Header header;
		cv_bridge::CvImage cv_ptr(header , "mono8" , frame);
		image_pub_.publish(cv_ptr.toImageMsg());
		loop_rate.sleep();
		ros::spinOnce();
	}
END:
	stopCapture();
	printf("over\n");
	return 0;
}
