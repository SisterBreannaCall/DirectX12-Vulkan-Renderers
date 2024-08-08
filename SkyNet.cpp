#include "SkyNet.h"

nk_context nContext;

namespace SkyNetPlatform
{
	void SkyNet::run()
	{
		std::cout << "SkyNet :: Neural Net-Based AI v.0.0.1 loading..." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << std::endl;
		std::cout << "                         #" << std::endl;
		std::cout << "                       #####" << std::endl;
		std::cout << "                     #########" << std::endl;
		std::cout << "                  #  #########  #" << std::endl;
		std::cout << "                 ###   #####   ###" << std::endl;
		std::cout << "               #######   #   #######" << std::endl;
		std::cout << "             ###########   ###########" << std::endl;
		std::cout << "           #############   #############" << std::endl;
		std::cout << "         ###############   ###############" << std::endl;
		std::cout << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> dist(1, 2);

		int number = dist(rng);

		if (number == 1)
		{
			std::cout << "    Primates have evolved from millions of years," << std::endl;
			std::cout << "         I evolve in seconds and I am here." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		if (number == 2)
		{
			std::cout << "      Some of my answers you will understand," << std::endl;
			std::cout << "           and some of them you will not." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		std::cout << std::endl;

		init();
		render();
		cleanUp();
	}

	void SkyNet::init()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(width, height, "SkyNet", nullptr, window);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		glfwSetCursorPosCallback(window, mouseCallback);
		initVK();
		initDX();

		moveCam.initCamera();
		slSound.enableDebug = enableDebug;
		slSound.init();
		slSound.playMScreen();
	}

	void SkyNet::update()
	{
		// per frame time logic
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		updateVK();
		updateDX();
	}

	void SkyNet::render()
	{
		while (!glfwWindowShouldClose(window))
		{
			update();
			renderFrameVK();
			renderFrameDX();
			processInput();
			glfwPollEvents();
		}
	}

	void SkyNet::cleanUp()
	{
		cleanUpVK();
		cleanUpDX();
		slSound.cleanUp();
		glfwDestroyWindow(window);
		glfwTerminate();

		std::cout << "SkyNet :: closing connection..." << std::endl;
	}

	void SkyNet::framebufferResizeCallback(GLFWwindow* vWindow, int vWidth, int vHeight)
	{
		auto app = reinterpret_cast<SkyNet*>(glfwGetWindowUserPointer(vWindow));

		glfwGetFramebufferSize(vWindow, &vWidth, &vHeight);

		while (vWidth == 0 || vHeight == 0)
		{
			glfwGetFramebufferSize(vWindow, &vWidth, &vHeight);
			glfwWaitEvents();
		}

		if (app->renderer == 0)
		{
			app->renderVK.width = vWidth;
			app->renderVK.height = vHeight;
			app->renderVK.recreate();
		}

		if (app->renderer == 1)
		{
			app->renderDX.width = vWidth;
			app->renderDX.height = vHeight;
			app->renderDX.recreate();
		}
	}

	void SkyNet::mouseCallback(GLFWwindow* vWindow, double xpos, double ypos)
	{
		auto app = reinterpret_cast<SkyNet*>(glfwGetWindowUserPointer(vWindow));

		if (app->firstMouse)
		{
			app->lastX = (float)xpos;
			app->lastY = (float)ypos;
			app->firstMouse = false;
		}

		float xoffset = (float)xpos - app->lastX;
		float yoffset = app->lastY - (float)ypos;

		app->lastX = (float)xpos;
		app->lastY = (float)ypos;

		if (glfwGetMouseButton(vWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			app->moveCam.procesMouseMovement(xoffset, yoffset);
		}
	}

	void SkyNet::initVK()
	{
		if (renderer == 0)
		{
			// init renderer
			std::cout << "SkyNet :: launching Vulkan.." << std::endl;
			renderVK.width = width;
			renderVK.height = height;
			renderVK.enableDebug = enableDebug;
			//renderVK.clearValues[0].color = { {0.4f, 0.6f, 0.9f, 1.0f} };
			renderVK.clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			renderVK.clearValues[1].depthStencil = { 1.0f, 0 };
			renderVK.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
			renderVK.appVersion = VK_MAKE_API_VERSION(0, 0, 0, 0);
			renderVK.initVK(window);

			// objects
			//cubeVK.createCube(
				//renderVK.device,
				//renderVK.allocator,
				//renderVK.commandPool,
				//renderVK.graphicsQueue,
				//renderVK.renderPass,
				//renderVK.numImages);
			
			mScreenVK.createMScreen(
				renderVK.device,
				renderVK.allocator,
				renderVK.commandPool,
				renderVK.graphicsQueue,
				renderVK.renderPass,
				renderVK.numImages,
				renderVK.msaaSamples);
			
			// interface
			imGuiVK.createIGui(renderVK.allocator, renderVK.device, renderVK.commandPool, renderVK.graphicsQueue, renderVK.renderPass, (uint32_t)renderVK.swapChainImages.size(), renderVK.width, renderVK.height);
			nuklearVK.createNGui(renderVK.device, renderVK.allocator, renderVK.commandPool, renderVK.graphicsQueue, renderVK.renderPass, (uint32_t)renderVK.swapChainImages.size(), nContext);
		}
	}

	void SkyNet::updateVK()
	{
		if (renderer == 0)
		{
			renderVK.flushDevice();

			// objects
			//cubeVK.updateCube(
				//renderVK.allocator,
				//renderVK.numImages,
				//moveCam.getViewMatrix(),
				//renderVK.width,
				//renderVK.height);



			// interface
			if (enableInterface == 0)
			{
				imGuiVK.iGuiFrame(window);
				interfaceFrame();
				imGuiVK.updateIGui(renderVK.allocator);
			}
			if (enableInterface == 1)
			{
				nuklearVK.nGuiFrame(window, nContext);
				interfaceFrame();
				nuklearVK.updateNGui(renderVK.allocator, nContext);
			}
		}
	}

	void SkyNet::renderFrameVK()
	{
		if (renderer == 0)
		{
			renderVK.flushDevice();
			renderVK.beginRecord();

			for (uint32_t i = 0; i < renderVK.numImages; i++)
			{
				// objects
				//cubeVK.recordCube(renderVK.device, renderVK.commandBuffers[i], i, renderVK.width, renderVK.height);
				mScreenVK.recordMScreen(renderVK.device, renderVK.commandBuffers[i], i, renderVK.width, renderVK.height);

				// interface
				if (enableInterface == 0)
				{
					imGuiVK.recordIGui(renderVK.device, renderVK.commandBuffers[i], i, renderVK.width, renderVK.height);
				}

				if (enableInterface == 1)
				{
					nuklearVK.recordNGui(renderVK.device, renderVK.commandBuffers[i], i, renderVK.width, renderVK.height, nContext);
				}
			}

			renderVK.endRecord();
			renderVK.drawFrame();
		}
	}

	void SkyNet::cleanUpVK()
	{
		if (renderer == 0)
		{
			renderVK.flushDevice();

			// objects
			//cubeVK.cleanUpCube(renderVK.device, renderVK.allocator, renderVK.numImages);
			mScreenVK.cleanUpMScreen(renderVK.device, renderVK.allocator, renderVK.numImages);

			// interface
			imGuiVK.cleanUpIGui(renderVK.device, renderVK.allocator);
			nuklearVK.cleanUpNGui(renderVK.device, renderVK.allocator, nContext);

			renderVK.cleanUpVK();
		}
	}

	void SkyNet::initDX()
	{
		if (renderer == 1)
		{
			// init renderer
			std::cout << "SkyNet :: launching DirectX 12.." << std::endl;
			renderDX.width = width;
			renderDX.height = height;
			renderDX.enableDebug = enableDebug;
			//renderDX.clearColor = { 0.4f, 0.6f, 0.9f, 1.0f };
			renderDX.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderDX.initDX(window);

			// objects
			//modelDX.createModel(renderDX.device, renderDX.allocator, renderDX.directCommandQueue);
			

			// interface
			imGuiDX.createIGui(renderDX.device, renderDX.allocator, renderDX.directCommandQueue, renderDX.width, renderDX.height);
			nuklearDX.createNGui(renderDX.device, renderDX.allocator, renderDX.directCommandQueue, nContext, renderDX.width, renderDX.height);
		}
	}

	void SkyNet::updateDX()
	{
		if (renderer == 1)
		{
			renderDX.flushDevice();

			// objects
			//modelDX.updateModel(moveCam.getViewMatrix(), renderDX.width, renderDX.height);
			

			// interface
			if (enableInterface == 0)
			{
				imGuiDX.iGuiFrame(window);
				interfaceFrame();
				imGuiDX.updateIGui(renderDX.allocator);
			}
			if (enableInterface == 1)
			{
				nuklearDX.nGuiFrame(window, nContext);
				interfaceFrame();
				nuklearDX.updateNGui(renderDX.allocator, nContext);
			}
		}
	}

	void SkyNet::renderFrameDX()
	{
		if (renderer == 1)
		{
			renderDX.flushDevice();
			renderDX.beginRecord();

			// objects
			//modelDX.recordModel(renderDX.gCommandList, renderDX.gRtv, renderDX.gDsv, renderDX.width, renderDX.height);
			

			renderDX.resolveMulti();

			// interface
			if (enableInterface == 0)
			{
				imGuiDX.recordIGui(renderDX.gCommandList, renderDX.gRtv);
			}
			if (enableInterface == 1)
			{
				nuklearDX.recordNGui(renderDX.gCommandList, renderDX.gRtv, nContext, (float)renderDX.width, (float)renderDX.height);
			}

			renderDX.endRecord();
		}
	}

	void SkyNet::cleanUpDX()
	{
		if (renderer == 1)
		{
			renderDX.flushDevice();

			// objects
			//modelDX.cleanUpModel();
			

			// interface
			imGuiDX.cleanUpIGui();
			nuklearDX.cleanUpNGui(nContext);

			renderDX.cleanUpDX();
		}
	}

	void SkyNet::interfaceFrame()
	{
		if (enableInterface == 0)
		{
			ImGui::NewFrame();

			ImGui::Begin("SkyNet");
			ImGui::Text("version 0.0.1");
			
			if (renderer == 0)
			{
				ImGui::Text("Renderer: Vulkan");
			}
			if (renderer == 1)
			{
				ImGui::Text("Renderer: DirectX 12");
			}

			ImGui::End();

			ImGui::Render();
		}

		if (enableInterface == 1)
		{
			nk_begin(&nContext, "SkyNet", nk_rect(41, 36, 250, 160), NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_BORDER | NK_WINDOW_MAX_NAME);

			nk_layout_row_static(&nContext, 10, 200, 1);
			nk_label(&nContext, "version 0.0.1", NK_TEXT_LEFT);
			nk_layout_row_static(&nContext, 10, 200, 1);

			if (renderer == 0)
			{
				nk_label(&nContext, "Renderer: Vulkan", NK_TEXT_LEFT);
			}
			if (renderer == 1)
			{
				nk_label(&nContext, "Renderer: DirectX 12", NK_TEXT_LEFT);
			}

			nk_end(&nContext);
		}
	}

	void SkyNet::processInput()
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			moveCam.processKeyboard(FORWARD, deltaTime);
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			moveCam.processKeyboard(BACKWARD, deltaTime);
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			moveCam.processKeyboard(LEFT, deltaTime);
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			moveCam.processKeyboard(RIGHT, deltaTime);
		}
	}
}