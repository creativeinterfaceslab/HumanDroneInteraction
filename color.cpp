#include <opencv2/opencv.hpp>
#include <bits/stdc++.h>
#include "PlutoPilot.h"
#include "App.h"
#include "Control.h"
#include "Led.h"

using namespace cv;
using namespace std;

int detect_color(Mat image, Scalar lower_bound, Scalar upper_bound) 
{
    Mat hsv;
    cvtColor(image, hsv, COLOR_BGR2HSV);
    
    Mat mask;
    inRange(hsv, lower_bound, upper_bound, mask);
    
    if (countNonZero(mask) > 0) 
    {
        return 1;
    } 
    else 
    {
        return 0;
    }
}

void onPilotStart()
{
  Control.disableFlightStatus(true);
  ledOp(L_RIGHT, ON);
  ledOp(L_MID, OFF);
  ledOp(L_LEFT, ON);
}

int main()
{
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Error: Couldn't access the camera." << endl;
        return -1;
    }

    while (true) {
        Mat frame;
        cap.read(frame);


        // Main logic for the color detection and functionality for the drone, you can initalise the drone class object and do the functionality
        int redDetected = detect_color(frame, Scalar(0, 100, 100), Scalar(10, 255, 255));
        int blueDetected = detect_color(frame, Scalar(110, 100, 100), Scalar(130, 255, 255));
        int greenDetected = detect_color(frame, Scalar(35, 100, 100), Scalar(85, 255, 255));

        if (redDetected) 
        {
            LED.set(RED, ON);
            LED.set(BLUE, OFF);
            LED.set(GREEN, OFF);
        } 
        else if (blueDetected) 
        {
            LED.set(RED, OFF);
            LED.set(BLUE, ON);
            LED.set(GREEN, OFF);
        } 
        else if (greenDetected) 
        {
            LED.set(RED, OFF);
            LED.set(BLUE, OFF);
            LED.set(GREEN, ON);
        } 
        else 
        {
            LED.set(RED, OFF);
            LED.set(BLUE, OFF);
            LED.set(GREEN, OFF);
        }
        
        imshow("frame", frame);
        
        if (waitKey(1) == 'q') 
        {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
}

void onPilotFinish()
{
	Control.disableFlightStatus(false);
}