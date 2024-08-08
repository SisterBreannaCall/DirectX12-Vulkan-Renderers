#include "MoveCam.h"

namespace SkyNetPlatform
{
	void MoveCam::initCamera()
	{
		updateCameraVectors();
	}

	void MoveCam::processKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = movementSpeed * deltaTime;
		
		if (direction == FORWARD)
		{
			position += front * velocity;
		}
		
		if (direction == BACKWARD)
		{
			position -= front * velocity;
		}
		
		if (direction == LEFT)
		{
			position -= right * velocity;
		}
		
		if (direction == RIGHT)
		{
			position += right * velocity;
		}
	}

	void MoveCam::procesMouseMovement(float xoffset, float yoffset)
	{
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}
		if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}
		
		updateCameraVectors();
	}

	glm::mat4 MoveCam::getViewMatrix()
	{
		return glm::lookAt(position, position + front, up);
	}

	glm::vec4 MoveCam::getCameraPosition()
	{
		return glm::vec4(position.x, position.y, position.z, 1.0f);
	}

	void MoveCam::updateCameraVectors()
	{
		glm::vec3 frontVar;
		frontVar.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		frontVar.y = sin(glm::radians(pitch));
		frontVar.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(frontVar);
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}
}