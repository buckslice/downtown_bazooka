

#include "physics.h"
#include <iostream>
#include <glm/gtx/projection.hpp>

Physics::Physics(Camera& cam) : cam(cam) {
}

void Physics::update(float delta) {
	// will eventually do this for each dynamic object
	glm::vec3 p = cam.pos;
	AABB player(glm::vec3(p.x - .5f, p.y - 1.8f, p.z - .5f), glm::vec3(p.x + .5f, p.y + .2f, p.z + .5f));

	AABB broadphase = player.getSwept(player, cam.vel);

	float time = 1.0f;
	glm::vec3 norm;
	GLfloat distCutoff = 200.0f;
	for (int i = 0; i < staticObjects.size(); i++) {
		// square distance check
		glm::vec3 diff = player.min - staticObjects[i].min;
		if (glm::dot(diff, diff) > distCutoff * distCutoff) {
			continue;
		}

		// broadphase sweep bounds check
		if (!AABB::check(broadphase, staticObjects[i])) {
			continue;
		}

		/*if (!AABB::check(player, staticObjects[i])) {
			std::cout << "BROADPHASE WORKING?" << std::endl;
		}*/

		// narrow sweep bounds check
		glm::vec3 n;
		float t = sweepTest(player, staticObjects[i], cam.vel*delta, n);
		if (t < time) {
			time = t;
			norm = n;
		}
	}

	cam.pos += cam.vel * delta * time;

	// slide along surface
	// need to recheck collisions after this since editing cam.pos might cause it to go through another object
	// happens when two buildings are very close to eachother
	// also fall through roof of buildings sometimes?
	// seems to only happen on small buildings that are wider than tall
	if (time < 1.0f) {
		glm::vec3 cvel = cam.vel - glm::proj(cam.vel, norm);
		//cam.vel -= glm::proj(cam.vel, norm);
		cam.pos += cvel * delta * (1.0f - time);
		if (norm.y != 0.0f) {
			//std::cout << "grounded " << timeSinceStart.getElapsedTime().asSeconds() << std::endl;
			cam.grounded = true;
			cam.vel.y = 0.0f;
		}
	}

	for (int i = 0; i < staticObjects.size(); i++) {
		if (AABB::check(player, staticObjects[i])) {
			std::cout << "thanks blizzard" << std::endl;
		}
	}

	// clamp camera to ground
	if (cam.pos.y < 1.8f) {
		cam.grounded = true;
		cam.pos.y = 1.8f;
		cam.vel.y = 0.0f;	// otherwise broadphase gets weird?
	}

}

void Physics::addObject(AABB& obj) {
	staticObjects.push_back(obj);
}

void Physics::addObjects(const std::vector<AABB>& objs) {
	for (int i = 0; i < objs.size(); i++) {
		staticObjects.push_back(objs[i]);
	}
}

void Physics::clearObjects() {
	staticObjects.clear();
}

float Physics::sweepTest(AABB b1, AABB b2, glm::vec3 vel, glm::vec3& norm) {
	// find distance between objects on near and far sides
	glm::vec3 invEntr;
	glm::vec3 invExit;

	if (vel.x > 0.0f) {
		invEntr.x = b2.min.x - b1.max.x;
		invExit.x = b2.max.x - b1.min.x;
	} else {
		invEntr.x = b2.max.x - b1.min.x;
		invExit.x = b2.min.x - b1.max.x;
	}
	if (vel.y > 0.0f) {
		invEntr.y = b2.min.y - b1.max.y;
		invExit.y = b2.max.y - b1.min.y;
	} else {
		invEntr.y = b2.max.y - b1.min.y;
		invExit.y = b2.min.y - b1.max.y;
	}
	if (vel.z > 0.0f) {
		invEntr.z = b2.min.z - b1.max.z;
		invExit.z = b2.max.z - b1.min.z;
	} else {
		invEntr.z = b2.max.z - b1.min.z;
		invExit.z = b2.min.z - b1.max.z;
	}

	// find time of collision
	glm::vec3 entr;
	glm::vec3 exit;

	if (vel.x == 0.0f) {
		entr.x = -std::numeric_limits<float>::infinity();
		exit.x = std::numeric_limits<float>::infinity();
	} else {
		entr.x = invEntr.x / vel.x;
		exit.x = invExit.x / vel.x;
	}
	if (vel.y == 0.0f) {
		entr.y = -std::numeric_limits<float>::infinity();
		exit.y = std::numeric_limits<float>::infinity();
	} else {
		entr.y = invEntr.y / vel.y;
		exit.y = invExit.y / vel.y;
	}
	if (vel.z == 0.0f) {
		entr.z = -std::numeric_limits<float>::infinity();
		exit.z = std::numeric_limits<float>::infinity();
	} else {
		entr.z = invEntr.z / vel.z;
		exit.z = invExit.z / vel.z;
	}

	float entrTime = glm::compMax(entr);
	float exitTime = glm::compMin(exit);
	/*float entrTime = glm::max(glm::max(entr.x, entr.y), entr.z);
	float exitTime = glm::min(glm::min(exit.x, exit.y), exit.z);*/

	//std::cout << entrTime << " " << exitTime << std::endl;

	//// if there was no collision
	//if (entrTime > exitTime ||
	//	entr.x < 0.0f && entr.y < 0.0f && entr.z < 0.0f ||
	//	entr.x > 1.0f || entr.y > 1.0f || entr.z > 1.0f) {
	//	norm = glm::vec3(0.0f);
	//	return 1.0f;
	//}
	if (entrTime > exitTime) {
		return 1.0f;
	}
	if (entr.x < 0.0f && entr.y < 0.0f && entr.z < 0.0f) {
		return 1.0f;
	}
	if (entr.x < 0.0f) {
		if (b1.max.x < b2.min.x || b1.min.x > b2.max.x) {
			return 1.0f;
		}
	}
	if (entr.y < 0.0f) {
		if (b1.max.y < b2.min.y || b1.min.y > b2.max.y) {
			return 1.0f;
		}
	}
	if (entr.z < 0.0f) {
		if (b1.max.z < b2.min.z || b1.min.z > b2.max.z) {
			return 1.0f;
		}
	}

	// calculate normal of collided surface
	if (entrTime == entr.x) {
		norm = glm::vec3(entr.x < 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
	} else if (entrTime == entr.y) {
		norm = glm::vec3(0.0f, entr.y < 0.0f ? 1.0f : -1.0f, 0.0f);
	} else {
		norm = glm::vec3(0.0f, 0.0f, entr.z < 0.0f ? 1.0f : -1.0f);
	}

	return entrTime;
}