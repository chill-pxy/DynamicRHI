#pragma once

#include <memory>
#include <iostream>

#include "Instance.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "SwapChain.h"
#include "CommandQueue.h"
#include "Texture.h"
#include "DescriptorPool.h"
#include "InterfaceType.h"
#include "Surface.h"
#include "NativeWindow.h"

namespace DRHI
{
	struct ContextCreatInfo
	{
		API api;
		const char* windowTitle;
		int windowWidth;
		int windowHeight;
	};

	class Context
	{
	private:
		API _runtimeInterface;
		const char* _windowTitle;
		int _windowWidth;
		int _windowHeight;

		std::unique_ptr<Instance> _instance;
		std::unique_ptr<Device> _device;
		std::unique_ptr<PhysicalDevice> _physicalDevice;
		std::unique_ptr<SwapChain> _swapChain;
		std::unique_ptr<CommandQueue> _graphicQueue;
		std::unique_ptr<CommandQueue> _presentQueue;
		std::unique_ptr<Surface> _surface;
		std::unique_ptr<NativeWindow> _nativeWindow;

	public:
		Context()
		{
			//Ĭ��ʹ��Vulkan
			_runtimeInterface = API::VULKAN;
			_windowTitle = "DefaultWindow";
			_windowWidth = 1920;
			_windowHeight = 1080;

			createMember();
		}

		Context(ContextCreatInfo info)
		{
			_runtimeInterface = info.api;
			_windowTitle = info.windowTitle;
			_windowWidth = info.windowWidth;
			_windowHeight = info.windowHeight;

			createMember();
		}

		void createMember()
		{
			_instance = std::make_unique<Instance>(_runtimeInterface);
			_physicalDevice = std::make_unique<PhysicalDevice>(_runtimeInterface);
			_device = std::make_unique<Device>(_runtimeInterface);
			_graphicQueue = std::make_unique<CommandQueue>(_runtimeInterface);
			_presentQueue = std::make_unique<CommandQueue>(_runtimeInterface);
			_swapChain = std::make_unique<SwapChain>(_runtimeInterface);
			_surface = std::make_unique<Surface>(_runtimeInterface);
			_nativeWindow = std::make_unique<NativeWindow>();
		}

		void initialize()
		{
			_nativeWindow->initialize(_windowTitle, _windowWidth, _windowHeight);
			
			_instance->createInstance(_nativeWindow->getNativeWindowExtensions());
			
			_surface->createSurface(_instance.get(), _nativeWindow->getNativeWindow());
			
			_physicalDevice->pickPhysicalDevice(0, _instance.get());
			_physicalDevice->pickGraphicQueueFamily();
			
			_device->createLogicalDevice(_physicalDevice.get(), _graphicQueue.get(), _presentQueue.get(), _surface.get());
			
			_swapChain->createSwapChain(_physicalDevice.get(), _device.get(), _surface.get(), _nativeWindow->getNativeWindow());
			_swapChain->createImageViews(_device.get());
		}

	};
}
