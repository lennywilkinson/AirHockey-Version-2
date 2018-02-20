#include "Graphics.h"

using namespace cv;

// Constructor, calculates conversion ratios, defaults hold time
Graphics::Graphics() {

	// calculate conversion ratios for table-to-graphics
	widthRatio_tableToGraphics = OUTPUT_IMAGE_WIDTH / table_width;
	heightRatio_tableToGraphics = OUTPUT_IMAGE_HEIGHT / table_height;

	// default hold time
	currentFrame_holdTime = 1;
}

// Prints a specified status message to the console
void Graphics::printStatusToConsole(std::string message) {

	// print simple status message
	std::cout << "STATUS: " << message << std::endl;
}

// Creates the game window, fullscreen or not
void Graphics::spawnWindow(bool makeFullscreen) {

	// spawn named window
	namedWindow(WINDOW_TITLE, CV_WINDOW_NORMAL);

	// check if fullscreen activated
	if(makeFullscreen)

		// make window fullscreen
		setWindowProperty(WINDOW_TITLE, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
}

// Attempts to import gameplay assets
bool Graphics::importResources(std::string path = "C:") {

// check OS, use proper syntax to attempt image imports
#ifdef _WIN32
	image_startupSplash = imread(path + "\\startupSplash.png");
	image_tableTop = imread(path + "\\tableTop.png");
	image_goalPlayerOne = imread(path + "\\goalPlayerOne.png");
	image_goalPlayerTwo = imread(path + "\\goalPlayerTwo.png");
	image_winPlayerOne = imread(path + "\\winPlayerOne.png");
	image_winPlayerTwo = imread(path + "\\winPlayerTwo.png");
	image_error = imread(path + "\\error.png");
#else
	image_startupSplash = imread(path + "//startupSplash.png");
	image_tableTop = imread(path + "//tableTop.png");
	image_goalPlayerOne = imread(path + "//goalPlayerOne.png");
	image_goalPlayerTwo = imread(path + "//goalPlayerTwo.png");
	image_winPlayerOne = imread(path + "//winPlayerOne.png");
	image_winPlayerTwo = imread(path + "//winPlayerTwo.png");
	image_error = imread(path + "//error.png");
#endif

	// check if any imports failed
	if (image_startupSplash.empty() || image_tableTop.empty() || image_goalPlayerOne.empty() || image_goalPlayerTwo.empty() || image_winPlayerOne.empty() || image_winPlayerTwo.empty() || image_error.empty())
		
		// report failure
		return false;

	// report success
	return true;
}

// Prints contents of memory buffer to screen
void Graphics::pushToScreen() {

	// check if hold time is remarkable (not 1)
	if (currentFrame_holdTime > 1) {

		// record frame start time
		auto frameTime = std::chrono::steady_clock::now();

		// iterate while time has not expired
		while (std::chrono::duration<long double, std::milli>(std::chrono::steady_clock::now() - frameTime).count() < currentFrame_holdTime) {
			
			// render buffer to screen, nudge CV to refresh
			imshow(WINDOW_TITLE, screenBuffer);
			waitKey(1);
		}
	}
	else {

		// render buffer to screen, nudge CV to refresh
		imshow(WINDOW_TITLE, screenBuffer);
		waitKey(1);
	}

	// reset hold time
	currentFrame_holdTime = 1;

	// reset buffer to error screen
	screenBuffer = image_error;
}

// Assembles the in-play game image
void Graphics::drawGameplayImage() {

	// copy table backdrop to buffer
	image_tableTop.copyTo(screenBuffer);

	// draw goal boxes to buffer
	rectangle(screenBuffer, Rect(Point2d((widthRatio_tableToGraphics*table_width) - (widthRatio_tableToGraphics * WALL_PADDING_THICKNESS), heightRatio_tableToGraphics * (table_height - GOAL_WIDTH) / 2.0), Point2d((widthRatio_tableToGraphics*table_width), heightRatio_tableToGraphics * (table_height + GOAL_WIDTH) / 2.0)), Scalar(0, 0, 0), -1);
	rectangle(screenBuffer, Rect(Point2d(0, heightRatio_tableToGraphics * (table_height - GOAL_WIDTH) / 2.0), Point2d((widthRatio_tableToGraphics * WALL_PADDING_THICKNESS), heightRatio_tableToGraphics * (table_height + GOAL_WIDTH) / 2.0)), Scalar(0, 0, 0), -1);


	// draw scores to buffer
	putText(screenBuffer, std::to_string(static_cast<int>(score_playerOne)), Point2d((widthRatio_tableToGraphics * table_width * 0.5) - 80, (heightRatio_tableToGraphics * table_height) - 60), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(50, 95, 105), 5);
	putText(screenBuffer, std::to_string(static_cast<int>(score_playerTwo)), Point2d((widthRatio_tableToGraphics * table_width * 0.5) + 40, (heightRatio_tableToGraphics * table_height) - 60), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(50, 95, 105), 5);

	// draw puck to buffer
	circle(screenBuffer, Point2d((puck_position[0] * widthRatio_tableToGraphics), (puck_position[1] * heightRatio_tableToGraphics)), PUCK_RADIUS * widthRatio_tableToGraphics, Scalar(10, 80, 10), -1);
	
	// draw paddle rings to buffer
	circle(screenBuffer, Point2d((paddleOne_position[0] * widthRatio_tableToGraphics), (paddleOne_position[1] * heightRatio_tableToGraphics)), PADDLE_RADIUS * widthRatio_tableToGraphics, Scalar(255, 0, 0), 5);
	circle(screenBuffer, Point2d((paddleTwo_position[0] * widthRatio_tableToGraphics), (paddleTwo_position[1] * heightRatio_tableToGraphics)), PADDLE_RADIUS * widthRatio_tableToGraphics, Scalar(0, 0, 255), 5);
	
	// set hold time (minimum)
	currentFrame_holdTime = 1;
}

// Creates the game startup image
void Graphics::drawStartupSplashImage() {

	// draw startup image to buffer
	screenBuffer = image_startupSplash;

	// set hold time (remarkable)
	currentFrame_holdTime = 3000;
}

// Creates a goal-scored screen for the specified player
void Graphics::drawGoalscoredImage(const bool wasPlayerOne) {

	// check which player
	if (wasPlayerOne)

		// player-one-score screen to buffer
		screenBuffer = image_goalPlayerOne;
	else

		// player-two-screen to buffer
		screenBuffer = image_goalPlayerTwo;

	// set hold time (remarkable)
	currentFrame_holdTime = 3000;
}

// Creates a game-won screen for the specified player
void Graphics::drawGamewonImage(const bool wasPlayerOne) {

	// check which player
	if (wasPlayerOne)

		// player-one-win screen to buffer
		screenBuffer = image_winPlayerOne;
	else

		// player-two-win screen to buffer
		screenBuffer = image_winPlayerTwo;

	// set hold time (remarkable)
	currentFrame_holdTime = 5000;
}