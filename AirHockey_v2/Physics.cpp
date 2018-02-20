#include "Physics.h"

using namespace std;

// Constructor, generates default positions and velocities from table dimensions
Physics::Physics() {

	// default puck position
	puck_position[0] = table_width / 2;
	puck_position[1] = table_height / 2;

	// default puck velocity (stopped)
	puck_velocity[0] = 0.0;
	puck_velocity[1] = 0.0;

	// default paddle positions approximately where real-world paddles should be
	paddleOne_position[0] = table_width / 4;
	paddleOne_position[1] = table_height/2;
	paddleTwo_position[0] = table_width * 3 / 4;
	paddleTwo_position[1] = table_height / 2;
}

// Conducts a full physics iteration (updates puck by velocity, updates paddle velocities)
void Physics::tick(const long double deltaTime_micros) {

	// update puck position (multiply velocity by elapsed time)
	puck_position[0] += puck_velocity[0] * deltaTime_micros * 1e-6;
	puck_position[1] += puck_velocity[1] * deltaTime_micros * 1e-6;

	// transpose puck velocity to polar coords
	double magnitude = sqrt(pow(puck_velocity[0], 2) + pow(puck_velocity[1], 2));
	double angle = atan2(puck_velocity[1], puck_velocity[0]);

	// apply friction force influence opposite to puck velocity
	// or, if velocity is very small, stop the puck
	if (magnitude > PUCK_FRICTION)
		magnitude -= (PUCK_FRICTION * deltaTime_micros * 1e-6);
	else
		magnitude = 0;

	// transpose coords back to cartesian
	puck_velocity[0] = magnitude * cos(angle);
	puck_velocity[1] = magnitude * sin(angle);

	// check if applying friction force would invert "horizontal" velocity
	if (abs(puck_velocity[0]) < (PUCK_FRICTION * deltaTime_micros * 1e-6))

		// make zero to prevent direction reversal
		puck_velocity[0] = 0;

	// check if applying friction force would invert "vertical" velocity
	if (abs(puck_velocity[1]) < (PUCK_FRICTION * deltaTime_micros * 1e-6))

		// make zero to prevent direction reversal
		puck_velocity[1] = 0;

	// deal with interactions
	handleCollisions();
}

// Handles puck bouncing off of paddles and walls, accounts for (but doesn't handle) goals
void Physics::handleCollisions() {

	// make sure interaction is not a goal
	if (puck_position[1] > (table_height + GOAL_WIDTH) / 2 || puck_position[1] < (table_height - GOAL_WIDTH) / 2) {

		// check if puck has collided with a "vertical" wall
		if (puck_position[0] <= PUCK_RADIUS + WALL_PADDING_THICKNESS || puck_position[0] >= (table_width - PUCK_RADIUS - WALL_PADDING_THICKNESS)) {

			// invert "horizontal" velocity, attenuate by elasticity
			puck_velocity[0] *= (-1.0 * WALL_ELASTICITY);

			// detect which wall puck intersects with
			if (puck_position[0] <= PUCK_RADIUS + WALL_PADDING_THICKNESS)

				// for "left" wall, move puck out of wall
				puck_position[0] = PUCK_RADIUS + 1 + WALL_PADDING_THICKNESS;
			else

				// for "right" wall, move puck out of wall
				puck_position[0] = table_width - PUCK_RADIUS - 1 - WALL_PADDING_THICKNESS;
		}

		// check if puck has collided with a "horizontal" wall
		if (puck_position[1] <= PUCK_RADIUS + WALL_PADDING_THICKNESS || puck_position[1] >= (table_height - PUCK_RADIUS - WALL_PADDING_THICKNESS)) {

			// invert "vertical" velocity, attenuate by elasticity
			puck_velocity[1] *= (-1.0 * WALL_ELASTICITY);

			// detect which wall puck intersects with
			if (puck_position[1] <= PUCK_RADIUS + WALL_PADDING_THICKNESS)

				// for "top" wall, move puck out of wall
				puck_position[1] = PUCK_RADIUS + 1 + WALL_PADDING_THICKNESS;
			else

				// for "bottom" wall, move puck out of wall
				puck_position[1] = table_height - PUCK_RADIUS - 1 - WALL_PADDING_THICKNESS;
		}
	}

	// create arrays for paddle position, velocity and index
	double* paddle_position;
	double* paddle_velocity;
	bool isPaddleOne;

	// check for puck collision with player one paddle
	if (hasCollision(true)) {

		// assign paddle one position, velocity and index as collision object
		paddle_position = paddleOne_position;
		paddle_velocity = paddleOne_velocity;
		isPaddleOne = true;
	}

	// check for puck collision with player two paddle
	else if (hasCollision(false)) {

		// assign paddle one position, velocity and index as collision object
		paddle_position = paddleTwo_position;
		paddle_velocity = paddleTwo_velocity;
		isPaddleOne = false;
	}

	// if no collision was detected, handleCollisions() is done, so stop
	else
		return;

	// calculate the position vector between the puck and the paddle
	double collisionNormal = atan2(puck_position[1] - paddle_position[1], puck_position[0] - paddle_position[0]);

	// calculate the impulse vector of the collision
	double collisionHeading = atan2(paddle_velocity[1] - puck_velocity[1], paddle_velocity[0] - puck_velocity[0]);

	// calculate the magnitude of force resulting from the interaction
	double magnitudeOfInteraction = sqrt(pow(puck_velocity[0] - paddle_velocity[0], 2) + pow(puck_velocity[1] - paddle_velocity[1], 2));

	// calculate the resulting velocity vector of the puck
	double bounceHeading = collisionNormal + (collisionNormal - collisionHeading);

	// transpose resulting vector from polar to cartesian
	puck_velocity[0] = magnitudeOfInteraction * cos(bounceHeading) + paddle_velocity[0];
	puck_velocity[1] = magnitudeOfInteraction * sin(bounceHeading) + paddle_velocity[1];

	// determine if puck intersects with paddle
	while (hasCollision(isPaddleOne)) {

		// move puck along resultant vector until it no longer intersects
		puck_position[0] += cos(collisionNormal);
		puck_position[1] += sin(collisionNormal);
	}
}

// Determines whether the puck intersects with the specified paddle
bool Physics::hasCollision(const bool isPaddleOne) {

	// check which paddle was requested
	if(isPaddleOne)

		// check for puck intersection with paddle one
		return (sqrt(pow(puck_position[0] - paddleOne_position[0], 2) + pow(puck_position[1] - paddleOne_position[1], 2)) <= (PUCK_RADIUS + PADDLE_RADIUS));
	else

		// check for puck intersection with paddle two
		return (sqrt(pow(puck_position[0] - paddleTwo_position[0], 2) + pow(puck_position[1] - paddleTwo_position[1], 2)) <= (PUCK_RADIUS + PADDLE_RADIUS));
}

// Determines whether the puck is in a goal
int Physics::detectGoals() {

	// check if the puck has a non-viable "vertical" coordinate
	if (puck_position[1] > (table_height + GOAL_WIDTH) / 2 || puck_position[1] < (table_height - GOAL_WIDTH) / 2)

		// report no goal
		return 0;

	// check if puck intersects with "left" goal
	if (puck_position[0] <= PUCK_RADIUS + WALL_PADDING_THICKNESS)

		// report player two goal
		return 2;

	// check if puck intersects with "right" goal
	if (puck_position[0] >= table_width - PUCK_RADIUS - WALL_PADDING_THICKNESS)

		// report player one goal
		return 1;

	// report no goal
	return 0;
}

// returns the puck to the given location and stops puck
void Physics::resetPuck(const double* new_position) {

	// update puck position to new coords
	puck_position[0] = new_position[0];
	puck_position[1] = new_position[1];

	// make puck velocity vector zero
	puck_velocity[0] = 0.0;
	puck_velocity[1] = 0.0;
}