#include <raspicam/raspicam_cv.h>

/*
 * Copyright (c) 2011. Philipp Wagner <bytefish[at]gmx[dot]de>.
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 */

#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include "wiringPi.h"

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>

using namespace cv;
using namespace cv::face;
using namespace std;

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
	std::ifstream file(filename.c_str(), ifstream::in);
	if (!file) {
		string error_message = "No valid input file was given, please check the given filename.";
		CV_Error(CV_StsBadArg, error_message);
	}
	string line, path, classlabel;
	int counter = 0;
	while (getline(file, line)) {
		cout << "Loading face " << counter/10 << " image " << counter%10 << endl;
		counter++;
		stringstream liness(line);
		getline(liness, path, separator);
		getline(liness, classlabel);
		if(!path.empty() && !classlabel.empty()) {
			images.push_back(imread(path, 0));
			labels.push_back(atoi(classlabel.c_str()));
		}
	}
}

int main(int argc, const char *argv[]) {
	// Check for valid command line arguments, print usage
	// if no arguments were given.
	/*
	   if (argc != 4) {
	   cout << "usage: " << argv[0] << " </path/to/haar_cascade> </path/to/csv.ext> </path/to/device id>" << endl;
	   cout << "\t </path/to/haar_cascade> -- Path to the Haar Cascade for face detection." << endl;
	   cout << "\t </path/to/csv.ext> -- Path to the CSV file with the face database." << endl;
	   cout << "\t <device id> -- The webcam device id to grab frames from." << endl;
	   exit(1);
	   }
	 */
	// Get the path to your CSV:
	string fn_haar = "/home/pi/opencv-3.4.1/data/haarcascades/haarcascade_frontalface_default.xml";
	string fn_lbp = "/home/pi/opencv-3.4.1/data/lbpcascades/lbpcascade_frontalface_improved.xml";
	string fn_csv = "/home/pi/Desktop/CameramanJohn/faces.txt";
	int deviceId = 0;

	// This sets up the GPIO Pins using wiring Pi
	wiringPiSetup();

	// This sets up the control pins for the stepper motors.
	int controlPin[4] = {0,1,2,3};  // Sets up the array
	pinMode(controlPin[0], OUTPUT); // Pin #17
	pinMode(controlPin[1], OUTPUT); // Pin #18
	pinMode(controlPin[2], OUTPUT); // Pin #27
	pinMode(controlPin[3], OUTPUT); // Pin #22

	// Hardcoded half-step sequence for a stepper motor
	int seqR[8][4] = {{1,0,0,0},
		{1,1,0,0},
		{0,1,0,0},
		{0,1,1,0},
		{0,0,1,0},
		{0,0,1,1},
		{0,0,0,1},
		{1,0,0,1}};
	int seqL[8][4] = {{0,0,0,1},
		{0,0,1,1},
		{0,0,1,0},
		{0,1,1,0},
		{0,1,0,0},
		{1,1,0,0},
		{1,0,0,0},
		{1,0,0,1}};

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

	// Get the height from the first image. We'll need this
	// later in code to reshape the images to their original
	// size AND we need to reshape incoming faces to this size:
	int im_width = images[0].cols;
	int im_height = images[0].rows;

	// Create a FaceRecognizer and train it on the given images:
	//		Ptr<FaceRecognizer> model = FisherFaceRecognizer::create();
	Ptr<FaceRecognizer> model = LBPHFaceRecognizer::create(); 

	// Training the face classifier
	cout << "Training..." << endl;
	model->train(images, labels);
	cout << "Done training" << endl;

	//This is the haar_cascade code
	//		CascadeClassifier haar_cascade;	
	//		haar_cascade.load(fn_haar);

	//This is the lbp cascade code
	CascadeClassifier lbp_cascade;
	lbp_cascade.load(fn_lbp);

	// Get a handle to the Video device:
	VideoCapture cap(deviceId);

	// Check if we can use this device at all:
	if(!cap.isOpened()) {
		cerr << "Capture Device ID " << deviceId << "cannot be opened." << endl;
		return -1;
	}
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);  //	X value
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // Y value
	
	//Creates a confidence variable to compare to
	double global_conf = std::numeric_limits<float>::infinity();;
	
	//Creates a variable to track image saving
	int total = 0;

	// Holds the current frame from the Video device:
	Mat frame;	
	for(;;) {
		cap >> frame;
		// Clone the current frame:
		Mat original = frame.clone();
		// Convert the current frame to grayscale:
		Mat gray;
		// cvtColor(frame, gray, CV_BGR2GRAY);
		cvtColor(original, gray, CV_BGR2GRAY);
		// Find the faces in the frame:
		vector< Rect_<int> > faces;

		// Calling the specific facial reconigzer and their detect function
		lbp_cascade.detectMultiScale(gray, faces);
		// haar_cascade.detectMultiScale(gray, faces);

		// At this point you have the position of the faces
		for(int i = 0; i < faces.size(); i++) {
			// Process face by face
			Rect face_i = faces[i];

			// Crop the face from the image.
			Mat face = gray(face_i);
			Mat face_resized;
			cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);
			
			// Saves the face
//			string destination("../faces/orl_faces/s41/kevin"+to_string(total)+".pgm");
//			imwrite(destination, face_resized);

			//Checking the confidence of the face
			int label = -1;
			double confidence = 0.0;
			model->predict(face_resized, label, confidence);

			if(confidence <= global_conf + 30)
			{
				global_conf = confidence;
				//Displaying face prediction
				rectangle(original, face_i, CV_RGB(255, 0,0), 1);

				//Calculate position of target
				int pos_x = face_i.tl().x+face_i.width/2;
				int pos_y = face_i.tl().y+face_i.height/2;

				// And now put it into the image:
				circle(original, Point(pos_x, pos_y), 1.0, CV_RGB(255,0,0), 2.0);

				//Calculate the corner of the rectangle
				int text_pos_x = std::max(face_i.tl().x - 10, 0);
				int text_pos_y = std::max(face_i.tl().y - 10, 0);

				//Display coords
				string boxtext = format("x=%d y=%d confidence=%f", pos_x, pos_y, confidence);
				putText(original, boxtext, Point(text_pos_x, text_pos_y),FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,0,0), 2.0);

				//Rotates the camera
				int center_x = 320;
				pid_t pid = fork();
				if(pid > 0){
					bool clock = (pos_x > center_x);
					int ticks = abs(center_x - pos_x);
					for(int i = 0; i < ticks; i++){
						for(int half = 0; half < 8; half++){
							for(int pin = 0; pin < 4; pin++){
								if(clock)
									digitalWrite(controlPin[pin], seqR[half][pin]);
								else
									digitalWrite(controlPin[pin], seqL[half][pin]);
							}
							delay(1);
						}
					}
					exit(0);
				}
			}
			total++;
		}
		// Show the result:
		imshow("face_recognizer", original);
		// And display it:
		char key = (char) waitKey(20);
		// Exit this loop on escape:
		if(key == 27)
			break;
	}
	return 0;
}
