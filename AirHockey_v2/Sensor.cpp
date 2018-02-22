#include "Sensor.h"

using namespace cv;

// returns the distance between two 2D points
double distance(cv::Point2d a, cv::Point2d b) {

	// report simple distance formula output
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// Constructor, initializes IR sensor and flare detection
Sensor::Sensor(const int port) {

	// open port to IR sensor
	sensor_ir = *(new VideoCapture(port));

	// create empty image container
	Mat setupImage;

	// fill with image from sensor buffer
	sensor_ir >> setupImage;

	// gather image dimensions for mapping calculations
	sensorFrame_width = setupImage.cols;
	sensorFrame_height = setupImage.rows;

	// preconfigure and create flare detector
	SimpleBlobDetector::Params sensorDetectionEngineParameters;
	sensorDetectionEngineParameters.filterByArea = true;
	sensorDetectionEngineParameters.filterByCircularity = false;
	sensorDetectionEngineParameters.filterByColor = false;
	sensorDetectionEngineParameters.filterByConvexity = false;
	sensorDetectionEngineParameters.filterByInertia = false;
	sensorDetectionEngineParameters.minArea = 100 / pow(SENSOR_DOWNSAMPLE_RATIO, 2);
	sensorDetectionEngineParameters.maxArea = 10000 / pow(SENSOR_DOWNSAMPLE_RATIO, 2);
	sensorDetectionEngine = SimpleBlobDetector::create(sensorDetectionEngineParameters);
}

// Uses uS sensor to predict physical projection size
void Sensor::detectProjectionSize() {

	// TODO
	double projectorDistance = 1000;// TODO: use the uS sensor here

	// calculate table dimensions from spread and distance
	table_width = projectorDistance * PROJECTOR_SPREAD_HORIZ;
	table_height = projectorDistance * PROJECTOR_SPREAD_VERT;

	// calculate important table points
	table_center[0] = table_width / 2;
	table_center[1] = table_height / 2;
	table_centerLeft[0] = table_width / 4;
	table_centerLeft[1] = table_center[1];
	table_centerRight[0] = table_width * 3 / 4;
	table_centerRight[1] = table_center[1];

	// calculate sensor -> table conversion factors
	widthRatio_sensorToTable = table_width / sensorFrame_width;
	heightRatio_sensorToTable = table_height / sensorFrame_height;
}

// Pulls an image from camera buffer into memory buffer
void Sensor::collectFrameFromCamera() {

	// streams cam buffer to mem buffer
	sensor_ir >> bufferImage;
}


// Preprocesses image and detects flares
void Sensor::processFrame() {

	// create empty image containers for intermediate steps
	Mat alphaImage, dilatedAlphaImage, imageForDetector;

	// fill container with binary image to filer low brightness false positives
	inRange(bufferImage, Scalar(0, 0, 0), Scalar(240, 240, 240), alphaImage);

	// fill container with dilated version of alpha image to close gaps
	dilate(alphaImage, dilatedAlphaImage, 0, Point(-1, -1), 5);

	// check if downsampling ratio is remarkable (not 1)
	if (SENSOR_DOWNSAMPLE_RATIO > 1)

		// downsample image to reduce flare detection time
		resize(dilatedAlphaImage, imageForDetector, Size(sensorFrame_width / SENSOR_DOWNSAMPLE_RATIO, sensorFrame_height / SENSOR_DOWNSAMPLE_RATIO));
	else
		
		// move entire image to detector
		imageForDetector = dilatedAlphaImage;

	// run flare detection and record points
	sensorDetectionEngine->detect(imageForDetector, detectedPoints);
}

// Locates paddles and calculates live velocities
void Sensor::updatePaddles(const double deltaTime_micros) {

	// save former positions for velocity calculations
	double paddleOne_lastPosition[2] = { paddleOne_position[0], paddleOne_position[1] };
	double paddleTwo_lastPosition[2] = { paddleTwo_position[0], paddleTwo_position[1] };

	// check if downsample ratio was remarkable (not 1)
	if (SENSOR_DOWNSAMPLE_RATIO > 1)

		// iterate through detected points
		for (int i = 0; i < detectedPoints.size(); i++) {

			// transpose detected points to table space
			detectedPoints.at(i).pt.x *= SENSOR_DOWNSAMPLE_RATIO;
			detectedPoints.at(i).pt.y *= SENSOR_DOWNSAMPLE_RATIO;
		}

	// initialize index and value for shortest distance
	int index = -1;
	double min_dist = 1e10;

	// iterate through detected points
	for (int i = 0; i < detectedPoints.size(); i++)

		// check if distance is shorter than current shortest
		if (distance(detectedPoints.at(i).pt, Point2d(paddleOne_lastPosition[0], paddleOne_lastPosition[1])) < min_dist && detectedPoints.at(i).pt.x < (sensorFrame_width / 2)) {
			
			// record as new current shortest distance
			index = i;
			min_dist = distance(detectedPoints.at(i).pt, Point2d(paddleOne_lastPosition[0], paddleOne_lastPosition[1]));
		}

	// check if a viable point was found
	if (index != -1) {

		// set paddle one position to new location
		paddleOne_position[0] = detectedPoints.at(index).pt.x * widthRatio_sensorToTable;
		paddleOne_position[1] = detectedPoints.at(index).pt.y * heightRatio_sensorToTable;
	}

	// initialize index and value for shortest distance
	index = -1;
	min_dist = 1e10;

	// iterate through detected points
	for (int i = 0; i < detectedPoints.size(); i++)

		// check if distance is shorter than current shortest
		if (distance(detectedPoints.at(i).pt, Point2d(paddleTwo_lastPosition[0], paddleTwo_lastPosition[1])) < min_dist && detectedPoints.at(i).pt.x > (sensorFrame_width / 2)) {
			
			// record as new current shortest distance
			index = i;
			min_dist = distance(detectedPoints.at(i).pt, Point2d(paddleTwo_lastPosition[0], paddleTwo_lastPosition[1]));
		}

	// check if a viable point was found
	if (index != -1) {

		// set paddle two position to new location
		paddleTwo_position[0] = detectedPoints.at(index).pt.x * widthRatio_sensorToTable;
		paddleTwo_position[1] = detectedPoints.at(index).pt.y * heightRatio_sensorToTable;
	}

	// use previous and current positions to calculate near-instantaneous velocity
	paddleOne_velocity[0] = (paddleOne_position[0] - paddleOne_lastPosition[0]) / (deltaTime_micros / 1e6);
	paddleOne_velocity[1] = (paddleOne_position[1] - paddleOne_lastPosition[1]) / (deltaTime_micros / 1e6);
	paddleTwo_velocity[0] = (paddleTwo_position[0] - paddleTwo_lastPosition[0]) / (deltaTime_micros / 1e6);
	paddleTwo_velocity[1] = (paddleTwo_position[1] - paddleTwo_lastPosition[1]) / (deltaTime_micros / 1e6);
}