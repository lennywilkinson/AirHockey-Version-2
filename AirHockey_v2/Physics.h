#pragma once
#include "GameData.h"

using namespace std;

// Physics handling class, processes velocity incrementation and collisions, detects goals
class Physics {
public:

	// Constructor, generates default positions and velocities from table dimensions
	Physics();

	// Conducts a full physics iteration (updates puck by velocity, updates paddle velocities)
	void tick(const long double);

	// Handles puck bouncing off of paddles and walls, accounts for (but doesn't handle) goals
	void handleCollisions();

	// Determines whether the puck intersects with the specified paddle
	bool hasCollision(const bool);

	// Determines whether the puck is in a goal
	int detectGoals();

	// returns the puck to the given location and stops puck
	void resetPuck(const double*);
};