#pragma once

#include <cmath>

class Physics {
// static class for physics calculations
public:
	const float GRAVITY = -9.8f;

	static float finalVelocity1(float initialVelocity, float acceleration, float distance) {
		return sqrt(pow(initialVelocity, 2) + (2 * acceleration * distance));
	}

	static float finalVelocity2(float initialVelocity, float acceleration, float time) {
		return initialVelocity + (acceleration * time);
	}

	static float initialVelocity1(float finalVelocity, float acceleration, float distance) {
		return sqrt(pow(finalVelocity, 2) - (2 * acceleration * distance));
	}

	static float initialVelocity2(float finalVelocity, float acceleration, float time) {
		return finalVelocity - (acceleration * time);
	}

	static float distance1(float initialVelocity, float acceleration, float time) {
		return (initialVelocity * time) + 0.5 * acceleration * pow(time, 2);
	}

	static float distance2(float initialVelocity, float finalVelocity, float time) {
		return 0.5 * time * (initialVelocity + finalVelocity);
	}

	static float acceleration1(float initialVelocity, float finalVelocity, float time) {
		return (finalVelocity - initialVelocity) / time;
	}

	static float acceleration2(float initialVelocity, float finalVelocity, float distance) {
		return (pow(finalVelocity, 2) - pow(initialVelocity, 2)) / (2 * distance);
	}

	static float time1(float initialVelocity, float finalVelocity, float distance) {
		return 2 * distance / (initialVelocity + finalVelocity);
	}

	static float time2(float initialVelocity, float finalVelocity, float acceleration) {
		return (initialVelocity + finalVelocity) / acceleration;
	}

};