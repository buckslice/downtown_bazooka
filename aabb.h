#pragma once

#include <glm/glm.hpp>
#include "intersectData.h"
class AABB {
public:
	const glm::vec3 min;
	const glm::vec3 max;

	AABB(const glm::vec3& min, const glm::vec3& max) :
		min(min),
		max(max) {
	}

	IntersectData getIntersect(const AABB& other) const;

};