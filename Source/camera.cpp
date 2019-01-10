#include "camera.h"

Camera::Camera():
	projection(mat4(1.0)), pitch(0.0), yaw(0.0)
{
}

Camera::Camera(float fov, float aspect, float zNear, float zFar):
	pitch(0.0), yaw(0.0)
{
	projection = perspective(radians(fov), aspect, zNear, zFar);
}

mat4 Camera::getViewProjection() const
{
	return projection * getView();
}

mat4 Camera::getView() const
{
	quat rotation = quat(vec3(pitch, yaw, 0));
	return glm::lookAt(position, position+normalize(glm::rotate(rotation, vec3(0.0, 0.0, 1.0))), vec3(0.0, 1.0, 0.0));
}

void Camera::update(float deltaTime)
{
	vec3 direction = vec3(0.0);
	direction.z = float(moveForward-moveBackward);
	direction.x = float(moveLeft-moveRight);
	if (direction.z || direction.x) direction = normalize(direction);

	position += glm::rotate(quat(vec3(pitch, yaw, 0)), direction) * 10.0f * deltaTime;
}
