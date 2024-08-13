#pragma once

#include <memory>
#include <iostream>
#include <functional>

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
#include "RenderPass.h"

namespace DRHI
{
	struct ContextCreatInfo
	{
		API         api;
		GLFWwindow* window;
		std::vector<const char*> windowExtensions;
	};

	class Context
	{
	private:
		API                      _runtimeInterface;
		GLFWwindow*              _window;
		std::vector<const char*> _windowExtensions;
		void*                    _surfaceCreateFunction;

		std::unique_ptr<Instance>       _instance;
		std::unique_ptr<Device>         _device;
		std::unique_ptr<PhysicalDevice> _physicalDevice;
		std::unique_ptr<SwapChain>      _swapChain;
		std::unique_ptr<CommandQueue>   _graphicQueue;
		std::unique_ptr<CommandQueue>   _presentQueue;
		std::unique_ptr<Surface>        _surface;
		std::unique_ptr<NativeWindow>   _nativeWindow;
		std::unique_ptr<RenderPass>     _renderPass;
		std::unique_ptr<DescriptorPool> _descriptorPool;

	public:
		Context() = delete;

		Context(ContextCreatInfo info)
		{
			_runtimeInterface = info.api;
			_window           = info.window;
			_windowExtensions = info.windowExtensions;

			createMember();
		}

		void createMember()
		{
			_instance       = std::make_unique<Instance>(_runtimeInterface);
			_physicalDevice = std::make_unique<PhysicalDevice>(_runtimeInterface);
			_device         = std::make_unique<Device>(_runtimeInterface);
			_graphicQueue   = std::make_unique<CommandQueue>(_runtimeInterface);
			_presentQueue   = std::make_unique<CommandQueue>(_runtimeInterface);
			_swapChain      = std::make_unique<SwapChain>(_runtimeInterface);
			_surface        = std::make_unique<Surface>(_runtimeInterface);
			_renderPass     = std::make_unique<RenderPass>(_runtimeInterface);
			_descriptorPool = std::make_unique<DescriptorPool>(_runtimeInterface);
		}

		void initialize()
		{
			_instance->createInstance(_windowExtensions);
			 
			_surface->createSurface(_runtimeInterface, _instance.get(), _window);

			_physicalDevice->pickPhysicalDevice(0, _instance.get());
			_physicalDevice->pickGraphicQueueFamily();
			
			_device->createLogicalDevice(_physicalDevice.get(), _graphicQueue.get(), _presentQueue.get(), _surface.get());
			
			_swapChain->createSwapChain(_physicalDevice.get(), _device.get(), _surface.get(), _window);
			_swapChain->createImageViews(_device.get());

			_renderPass->createRenderPass(_swapChain.get(), _device.get(), _physicalDevice.get());
			
			_descriptorPool->createDescriptorSetLayout(_device.get());
			_descriptorPool->createDescriptorPool(_device.get());
		}

		

	};
}
