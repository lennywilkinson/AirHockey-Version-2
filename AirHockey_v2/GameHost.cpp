#pragma once
#include "GameHost.h"

// create graphics, physics and sensor instance references
Graphics *graphics;
Physics *physics;
Sensor *sensor;

// Main, handles setup and spawns physics, sensor and graphics threads
int main() {

	// record the starting time of the program
	auto startTime = std::chrono::steady_clock::now();

	// mark game as in play and set gameState
	game_in_play = true;
	setGameState(IN_PLAY);

	// create sensor instance and calibrate table size
	sensor = new Sensor(0);
	sensor->detectProjectionSize();
	
	// create physics instance
	physics = new Physics();

	// create graphics instance
	graphics = new Graphics();

	// attempt to import assets
	if (!graphics->importResources(ASSET_PATH)) {

		// report one or more missing asset(s)
		std::cout << "ERROR: Asset(s) Missing, Check Directory" << std::endl;

		// terminate program with failure
		return EXIT_FAILURE;
	}

	// create window
	graphics->spawnWindow(true);	// DEBUG ONLY, CHANGE TO true FOR PROD

	// draw startup image, refresh screen
	graphics->drawStartupSplashImage();
	graphics->pushToScreen();

	// set puck to middle of table
	physics->resetPuck(table_center);

	// spawn physics and sensor threads, graphics to be handled on main thread
	thread tPhysics(physicsThread);
	thread tSensor(sensorThread);
	
	// set game state and call graphics on main thread
	setGameState(IN_PLAY);
	graphicsThread();

	// wait for all threads to complete
	tSensor.join();
	tPhysics.join();

	// terminate program with success
	return EXIT_SUCCESS;
}

// Handles physics algorithm, acts as physics update loop
void physicsThread() {

	// initialize lastTime for deltaTime calculations
	auto lastTime = std::chrono::steady_clock::now();

	// initialize lastTime and nFrames for FPS counter
	auto lastTime_frameCounter = lastTime;
	int frames = 0;

	// iterate while the game is in play
	while (game_in_play) {

		// record current time for deltaTime and FPS clock
		auto currentTime = std::chrono::steady_clock::now();

		// compute deltaTime values (durations)
		std::chrono::duration<long double, std::micro> deltaTime = currentTime - lastTime;
		std::chrono::duration<double, std::micro> frameCountDur = currentTime - lastTime_frameCounter;

		// increment frame counter
		frames++;

		// check if frame counter should report
		if (frameCountDur.count() >= 5e6) {
			
			// report FPS average over previous five seconds
			std::cout << "Physics Framerate: " << frames / 5 << std::endl;

			// reset frame counter
			frames = 0;

			// reset FPS trigger clock
			lastTime_frameCounter = currentTime;
		}

		// check gameState
		if(getGameState() == IN_PLAY)

			// tick physics
			physics->tick(deltaTime.count());

		// check if a goal has been scored
		if (physics->detectGoals() > 0)

			// check if player one scored a goal
			if (physics->detectGoals() == 1)

				// increment score, check if player won game
				if (++score_playerOne >= WINNING_SCORE) {

					// change game state
					setGameState(WIN_ONE);

					// reset scores for new game
					score_playerOne = 0;
					score_playerTwo = 0;

					// reset puck to middle
					physics->resetPuck(table_center);
				}
				else {
					
					// change game state
					setGameState(GOAL_ONE);

					// reset puck to player two's side
					physics->resetPuck(table_centerRight);
				}
			else

				// increment score, check if player won game
				if (++score_playerTwo >= WINNING_SCORE) {

					// change game state
					setGameState(WIN_TWO);

					// reset scores for new game
					score_playerOne = 0;
					score_playerTwo = 0;

					// reset puck to middle
					physics->resetPuck(table_center);
				}
				else {

					// change game state
					setGameState(GOAL_TWO);

					// reset puck to player ones side
					physics->resetPuck(table_centerLeft);
				}
		
		// save time at which frame began
		lastTime = currentTime;
	}
}


// Handles graphics assembly, celebration screens and display
void graphicsThread() {

	// initialize lastTime for deltaTime calculations
	auto lastTime = std::chrono::steady_clock::now();

	// initialize lastTime and counter for FPS calculation
	auto lastTime_frameCounter = lastTime;
	int frames = 0;

	// iterate while game is in play
	while (game_in_play) {

		// record start of frame time
		auto currentTime = std::chrono::steady_clock::now();

		// calculate deltaTimes (durations)
		std::chrono::duration<double, std::micro> deltaTime = currentTime - lastTime;
		std::chrono::duration<double, std::micro> frameCountDur = currentTime - lastTime_frameCounter;
		
		// increment frames
		frames++;

		// check if FPS needs reporting
		if (frameCountDur.count() >= 5e6) {

			// report FPS average over five seconds
			std::cout << "Graphics Framerate: " << frames / 5 << std::endl;
			
			// reset frame counter
			frames = 0;

			// reset FPS trigger timing
			lastTime_frameCounter = currentTime;
		}

		// check if gameState needs resetting, record evaluation
		bool reset_state = (getGameState() != IN_PLAY);

		// check if game is in play
		if (getGameState() == IN_PLAY)

			// assemble game-in-play image
			graphics->drawGameplayImage();

		// check if player one has scored
		else if (getGameState() == GOAL_ONE)

			// create player-one-scored screen
			graphics->drawGoalscoredImage(true);

		// check if player two has scored
		else if (getGameState() == GOAL_TWO)

			// create player-two-scored screen
			graphics->drawGoalscoredImage(false);

		// check if player one has won
		else if (getGameState() == WIN_ONE)

			// create player-one-win screen
			graphics->drawGamewonImage(true);

		// check if player two has won
		else if (getGameState() == WIN_TWO)

			// create player-two-win screen
			graphics->drawGamewonImage(false);
		
		// move assembled frame from buffer to screen
		graphics->pushToScreen();
		
		// recall flag from before frame
		if(reset_state)

			// reset flag to indicate celebration screen was shown
			setGameState(IN_PLAY);

		// record time of frame start
		lastTime = currentTime;
	}
}

// Handles sensor frame gathering and position data extraction
void sensorThread() {

	// initialize lastTime for deltaTime calculations
	auto lastTime = std::chrono::steady_clock::now();

	// initialize lastTime and framecounter for FPS reporting
	auto lastTime_frameCounter = lastTime;
	int frames = 0;

	// iterate while game is in play
	while (game_in_play) {

		// record time of frame start
		auto currentTime = std::chrono::steady_clock::now();

		// calculate deltaTimes (durations)
		std::chrono::duration<double, std::micro> deltaTime = currentTime - lastTime;
		std::chrono::duration<double, std::micro> frameCountDur = currentTime - lastTime_frameCounter;
		
		// increment frames
		frames++;

		// check if FPS needs to be reported
		if (frameCountDur.count() >= 5e6) {

			// report FPS average over previous five seconds
			std::cout << "Sensor Framerate: " << frames / 5 << std::endl;
			
			// reset frame counter
			frames = 0;

			// reset FPS trigger timer
			lastTime_frameCounter = currentTime;
		}

		// pull image from sensor buffer into memory
		sensor->collectFrameFromCamera();

		// perform image processing and extract data
		sensor->processFrame();

		// update positions of paddles for physics and graphics processing
		sensor->updatePaddles(deltaTime.count());

		// preserve time of frame start
		lastTime = currentTime;
	}
}