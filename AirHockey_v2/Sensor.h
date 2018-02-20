#pragma once
#include "GameData.h"

// Sensor handling class, controls IR sensor and data extraction
class Sensor {
public:

	// Constructor, initializes IR sensor and flare detection
	Sensor(const int);

	// Uses uS sensor to predict physical projection size
	void detectProjectionSize();

	// Pulls an image from camera buffer into memory buffer
	void collectFrameFromCamera();

	// Preprocesses image and detects flares
	void processFrame();

	// Locates paddles and calculates live velocities
	void updatePaddles(const double);
private:

	// conversion ratios for sensor-space to table-space
	double widthRatio_sensorToTable;
	double heightRatio_sensorToTable;

	// input image frame dimensions
	double sensorFrame_width;
	double sensorFrame_height;

	// flare detection candidate point vector
	std::vector<cv::KeyPoint> detectedPoints;

	// memory space to store current frame
	cv::Mat bufferImage;

	// flare detector reference (uninitialied)
	cv::Ptr<cv::SimpleBlobDetector> sensorDetectionEngine;

	// sensor reference (uninitialized)
	cv::VideoCapture sensor_ir;
};
