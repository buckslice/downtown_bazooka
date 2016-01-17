#include "GameObject.h"

GameObject::GameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;
}

glm::vec3 GameObject::getPosition() {
	return position;
}

glm::vec3 GameObject::getRotation() {
	return rotation;
}

glm::vec3 GameObject::getScale() {
	return scale;
}

void GameObject::move(glm::vec3 newPosition) {
	position = newPosition;
}

void GameObject::rotate(glm::vec3 newRotation) {
	rotation = newRotation;
}

void GameObject::resize(glm::vec3 newScale) {
	scale = newScale;
}
