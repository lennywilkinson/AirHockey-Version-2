#pragma once
#include "GameData.h"
#include "Graphics.h"
#include "Physics.h"
#include "Sensor.h"

// game state flags
bool game_in_play = true;
int gameState = IN_PLAY;

// puck position and velocity
double puck_position[2] = { 0,0 }, puck_velocity[2] = { 0,0 };

// paddle positions and velocities
double paddleOne_position[2] = { 0,0 }, paddleOne_velocity[2] = { 0,0 };
double paddleTwo_position[2] = { 0,0 }, paddleTwo_velocity[2] = { 0,0 };

// table dimensions and important points
double table_width = 0, table_height = 0, table_center[2] = { 0,0 }, table_centerLeft[2] = { 0,0 }, table_centerRight[2] = { 0,0 };

// game score
int score_playerOne = 0, score_playerTwo = 0;

// Main, handles setup and spawns physics, sensor and graphics threads
int main();

// Handles physics algorithm, acts as physics update loop
void physicsThread();

// Handles graphics assembly, celebration screens and display
void graphicsThread();

// Handles sensor frame gathering and position data extraction
void sensorThread();

// thread cross-modification protection instance
std::mutex mu;

// thread-safe gameState flag modifier
void setGameState(int new_state) {
	mu.lock();
	gameState = new_state;
	mu.unlock();
}

// thread-safe gameState flag retriever
int getGameState() {
	return gameState;
}