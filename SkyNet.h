#ifndef SKYNET_H
#define SKYNET_H

#include <Windows.h>
#include "RenderVK.h"
#include "RenderDX.h"
#include "SLSound.h"
#include "ImGuiVK.h"
#include "NuklearVK.h"
#include "ImGuiDX.h"
#include "NuklearDX.h"
#include "MoveCam.h"
#include "MScreenVK.h"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

namespace SkyNetPlatform
{
	class SkyNet
	{
	private:
		GLFWwindow* window;
		int width = 800;
		int height = 600;
		bool enableDebug = true;

		// 0 for VK or 1 for DX
		int renderer = 0;
		int enableInterface = 0;

		// camera
		float lastX = width / 2.0f;
		float lastY = height / 2.0f;
		bool firstMouse = true;
		float deltaTime = 0.0f;
		float lastFrame = 0.0f;

		Vulkan::RenderVK renderVK;
		DirectX::RenderDX renderDX;
		SoL::SLSound slSound;
		Vulkan::ImGuiVK imGuiVK;
		Vulkan::NuklearVK nuklearVK;
		DirectX::ImGuiDX imGuiDX;
		DirectX::NuklearDX nuklearDX;
		MoveCam moveCam;
		Vulkan::MScreenVK mScreenVK;

		static void framebufferResizeCallback(GLFWwindow* vWindow, int vWidth, int vHeight);
		static void mouseCallback(GLFWwindow* vWindow, double xpos, double ypos);

		void init();
		void update();
		void render();
		void cleanUp();
		void processInput();
		void interfaceFrame();

		void initVK();
		void updateVK();
		void renderFrameVK();
		void cleanUpVK();
		void initDX();
		void updateDX();
		void renderFrameDX();
		void cleanUpDX();

	public:
		void run();
	};
}
#endif