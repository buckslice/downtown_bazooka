#include "aabb.h"
#include "glm/gtx/component_wise.hpp"

IntersectData AABB::getIntersect(const AABB& other) const {
	// #thanks benny
	float maxDistance = glm::compMax(glm::max(other.min-max, min-other.max));
	return IntersectData(maxDistance < 0, maxDistance);
}