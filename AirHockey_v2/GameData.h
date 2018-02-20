#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>

// check OS, include OpenCV headers and asset path with proper file path format
#ifdef _WIN32
	#include <opencv2\highgui.hpp>
	#include <opencv2\features2d.hpp>
	#include <opencv2\imgproc.hpp>
	#include <opencv2\core.hpp>
	#define ASSET_PATH "assets\\"
#else
	#include <opencv2/highgui.hpp>
	#include <opencv2/features2d.hpp>
	#include <opencv2/imgproc.hpp>
	#include <opencv2/core.hpp>
	#define ASSET_PATH "assets//"
#endif


// define parameters for gameplay tweaking
#define PROJECTOR_SPREAD_HORIZ 1.0		// (width of projected image)/(distance from projector)
#define PROJECTOR_SPREAD_VERT 2.0/3.0	// (height of projected image)/(distance from projector)
#define OUTPUT_IMAGE_WIDTH 1200.0		// pixelwise width of projection
#define OUTPUT_IMAGE_HEIGHT 800.0		// pixelwise height of projection
#define SENSOR_DOWNSAMPLE_RATIO 1		// higher for less accurate but faster blob detection
#define PUCK_RADIUS 25.0				// radius of puck in real-world relative units
#define PADDLE_RADIUS 35.0				// same for paddles
#define WALL_PADDING_THICKNESS 18.0		// use to account for padding in bg image
#define GOAL_WIDTH 150.0				// width in real-world relative units
#define PUCK_MAX_VELOCITY 1e9			// threshold to catch unrealistic physics
#define WALL_ELASTICITY 0.8				// coefficient of energy conserved during collision
#define PUCK_FRICTION 50.0				// units per sec of deceleration
#define WINNING_SCORE 10				// score to win
#define PHYSICS_FRAME_RATIO 100			// number of physics frames per graphics frame (increase to resolve high speed collisions)
#define GRAPHICS_TARGET_FRAMERATE 30	// target framerate for display (NOT detection)
#define GOAL_CELEBRATION_TIME 3000		// time (ms) to display goal splash
#define WINDOW_TITLE "Virtual Air Hockey"

// gameplay-state flag states
#define SETUP -1
#define IN_PLAY 0
#define GOAL_ONE 1
#define GOAL_TWO 2
#define WIN_ONE 3
#define WIN_TWO 4
#define ERROR 5

// game state flags
extern bool game_in_play;
extern int gameState;

// puck position and velocity arrays
extern double puck_position[2], puck_velocity[2];

// paddle position and velocity arrays
extern double paddleOne_position[2], paddleOne_velocity[2];
extern double paddleTwo_position[2], paddleTwo_velocity[2];

// table dimension information
extern double table_width, table_height, table_center[2], table_centerLeft[2], table_centerRight[2];

// game scores
extern int score_playerOne, score_playerTwo;