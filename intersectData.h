#pragma once

// this class stores information about two intersecting objects
class IntersectData {
public:
	const bool doesIntersect;
	const float distance;

	IntersectData(const bool doesIntersect, const float distance) :
		distance(distance),
		doesIntersect(doesIntersect) {
	}
};