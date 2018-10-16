#include <sstream>
#include <fstream>
#include <iostream>

#include <raspicam/raspicam_cv.h>

#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/face.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace cv::face;
using namespace std; 

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
		std::ifstream file(filename.c_str(), ifstream::in);
		if (!file) {
				string error_message = "No valid input file was given,please check the given filename.";
				CV_Error(CV_StsBadArg, error_message);
		}
		string line, path, classlabel;
		while (getline(file, line)) {
				stringstream liness(line);
				getline(liness, path, separator);
				getline(liness, classlabel);
				if(!path.empty() && !classlabel.empty()) {
						images.push_back(imread(path, 0));
						labels.push_back(atoi(classlabel.c_str()));
				}
		}
}

int main ( int argc,char **argv ) {
		// Check for valid command line arguments, print usage
		// if no arguments were given.
		if (argc != 4) {
				cout << "usage: " << argv[0] << " </path/to/haar_cascade> </path/to/csv.ext> </path/to/device id>" << endl;
				cout << "\t </path/to/haar_cascade> -- Path to the Haar Cascade for face detection." << endl;
				cout << "\t </path/to/csv.ext> -- Path to the CSV file with the face database." << endl;
				cout << "\t <device id> -- The webcam device id to grab frames from." << endl;
				exit(1);
		}
		// Get the path to your CSV:
		string fn_haar = string(argv[1]);
		string fn_csv = string(argv[2]);
		int deviceId = atoi(argv[3]);
		// These vectors hold the images and corresponding labels:
		vector<Mat> images;
		vector<int> labels;
		// Read in the data (fails if no valid input filename is given, but you'll get an error message):
		try {
				read_csv(fn_csv, images, labels);
		} catch (cv::Exception& e) {
				cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
				// nothing more we can do
				exit(1);
		}

		//Face recognition variables
		Ptr<FaceRecognizer> model = FisherFaceRecognizer::create();
		model->train(images, labels);
		CascadeClassifier haar_cascade;
		haar_cascade.load(fn_haar);
		int im_width = images[0].cols;
		int im_height = images[0].rows;

		//Camera/image
		raspicam::RaspiCam_Cv Camera;
		cv::Mat image;
		//set camera params
		Camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
		//Open camera
		cout<<"Opening Camera..."<<endl;
		if (!Camera.open()) {cerr<<"Error opening the camera"<<endl;return -1;}

		//Start Display
		namedWindow("Window", WINDOW_AUTOSIZE);	

		//Start capture
		cout<<"Capturing..."<<endl;
		while (true) {
				Camera.grab();
				Camera.retrieve (image);

				//Cloning the image and converting it to grayscale
				Mat copyImg = image.clone();
				Mat grayImg;
				cvtColor(copyImg, grayImg, CV_BGR2GRAY);

				vector< Rect_<int> > faces;
				haar_cascade.detectMultiScale(grayImg, faces);
				// At this point you have the position of the faces in
				// faces. Now we'll get the faces, make a prediction and
				// annotate it in the video. Cool or what?
				for(int i = 0; i < faces.size(); i++) {
						// Process face by face:
						Rect face_i = faces[i];
						// Crop the face from the image. So simple with OpenCV C++:
						Mat face = grayImg(face_i);
						
						//Resizing the image
						Mat face_resized;
						cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);
						
						// Now perform the prediction, see how easy that is:
						int prediction = model->predict(face_resized);
						
						// And finally write all we've found out to the original image!
						// First of all draw a green rectangle around the detected face:
						rectangle(copyImg, face_i, CV_RGB(0, 255,0), 1);
						// Create the text we will annotate the box with:
						string box_text = format("Prediction = %d", prediction);
						// Calculate the position for annotated text (make sure we don't
						// put illegal values in there):
						int pos_x = std::max(face_i.tl().x - 10, 0);
						int pos_y = std::max(face_i.tl().y - 10, 0);
						// And now put it into the image:
						putText(copyImg, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
				}

				imshow("Window", image);
				char key = (char) waitKey(20);
				if(key == 27)
						break;
		}
		cout<<"Stop camera..."<<endl;
		Camera.release();
		cout<<"Closing program..."<<endl;
		
}

