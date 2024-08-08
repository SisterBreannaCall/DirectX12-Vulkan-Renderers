#ifndef MOVECAM_H
#define MOVECAM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SkyNetPlatform
{
	enum Camera_Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	class MoveCam
	{
	private:
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
		glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right;
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		float yaw = -90.0f;
		float pitch = 0.0f;

		float movementSpeed = 5.0f;
		float mouseSensitivity = 0.1f;

		void updateCameraVectors();

	public:
		void initCamera();
		glm::mat4 getViewMatrix();
		glm::vec4 getCameraPosition();
		void processKeyboard(Camera_Movement direction, float deltaTime);
		void procesMouseMovement(float xoffset, float yoffset);
	};
}
#endif