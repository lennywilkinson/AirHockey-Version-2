#pragma once
#include "GameData.h"
// Graphics handling class, assembles gameplay image and celebration screens, etc.
class Graphics {
public:

	// Constructor, calculates conversion ratios, defaults hold time
	Graphics();

	// Prints a specified status message to the console
	void printStatusToConsole(std::string message);

	// Creates the game window, fullscreen or not
	void spawnWindow(bool);

	// Attempts to import gameplay assets
	bool importResources(std::string);

	// Prints contents of memory buffer to screen
	void pushToScreen();

	// Assembles the in-play game image
	void drawGameplayImage();

	// Creates the game startup image
	void drawStartupSplashImage();

	// Creates a goal-scored screen for the specified player
	void drawGoalscoredImage(const bool);

	// Creates a game-won screen for the specified player
	void drawGamewonImage(const bool);

private:

	// conversion ratios for table-space to screen-space
	double widthRatio_tableToGraphics;
	double heightRatio_tableToGraphics;

	// time for the next rendered frame to be held on-screen for
	int currentFrame_holdTime;

	// master memory buffer, staging area for screen image
	cv::Mat screenBuffer;

	// game background/message images, constant after import
	cv::Mat image_startupSplash;
	cv::Mat image_tableTop;
	cv::Mat image_goalPlayerOne;
	cv::Mat image_goalPlayerTwo;
	cv::Mat image_winPlayerOne;
	cv::Mat image_winPlayerTwo;
	cv::Mat image_error;
};