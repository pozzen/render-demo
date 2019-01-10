#ifndef _CAMERA_H_INCLUDED_
#define _CAMERA_H_INCLUDED_

#include "main.h"

class Camera
{
public:
	Camera();
	Camera(float fov, float aspect, float zNear, float zFar);
	mat4 getViewProjection() const;
	mat4 getView() const;
	void update(float deltaTime);

	mat4 projection;
	vec3 position;
	float pitch;
	float yaw;

	bool moveForward = false;
	bool moveBackward = false;
	bool moveLeft = false;
	bool moveRight = false;
};

#endif // _CAMERA_H_INCLUDED_
