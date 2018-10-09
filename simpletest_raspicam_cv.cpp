#include <ctime>
#include <iostream>
#include <raspicam/raspicam_cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace std; 
using namespace cv;
int main ( int argc,char **argv ) {

		time_t timer_begin,timer_end;
		raspicam::RaspiCam_Cv Camera;
		cv::Mat image;
		int nCount=100;
		//set camera params
		Camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 );
		//Open camera
		cout<<"Opening Camera..."<<endl;
		if (!Camera.open()) {cerr<<"Error opening the camera"<<endl;return -1;}
		//Start capture
		cout<<"Capturing "<<nCount<<" frames ...."<<endl;
		time ( &timer_begin );
		for ( int i=0; i<nCount; i++ ) {
				Camera.grab();
				Camera.retrieve ( image);
				if ( i%5==0 )  cout<<"\r captured "<<i<<" images"<<std::flush;
		}
		cout<<"Stop camera..."<<endl;
		Camera.release();
		//show time statistics
		time ( &timer_end ); /* get current time; same as: timer = time(NULL)  */
		double secondsElapsed = difftime ( timer_end,timer_begin );
		cout<< secondsElapsed<<" seconds for "<< nCount<<"  frames : FPS = "<<  ( float ) ( ( float ) ( nCount ) /secondsElapsed ) <<endl;
		//display the image
		namedWindow("Displaywindow", WINDOW_AUTOSIZE);
		imshow("DisplayWindow", image);
		cout<<"displaying image"<<endl;

		//save image 
		cv::imwrite("raspicam_cv_image.jpg",image);
		cout<<"Image saved at raspicam_cv_image.jpg"<<endl;
		waitKey(0);
		return 0;
}
