#ifndef DEVICE_H
#define DEVICE_H

#include "vulkan/vulkan.h"

#include <vector>
#include <optional>
#include "window.h"
#include "swapchain.h"



namespace gigno {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() {
			return graphicFamily.has_value() && presentFamily.has_value();
		}
	};

	class Device {

	public:
		Device() = default;
		~Device();
		Device(const Device &) = delete;
		Device & operator=(const Device &) = delete;

		void Init(const Window *window);

		VkDevice GetDevice() const { return m_VkDevice; }
		VkInstance GetInstance() const { return m_VkInstance; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }
		SwapChainSupportDetails GetPhysicalSwapChainSupport() const { return QuerySwapChainSupport(m_PhysicalDevice); }
		QueueFamilyIndices GetPhysicalDeviceQueueFamilyIndices() const { return FindQueueFamiliyIndices(m_PhysicalDevice); }
		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }

	private :
		void CreateInstance();
		void CreateDebugMessenger();
		void PickPhysicalDevice();
		void CreateVulkanDevice();

//--------------------------------------------------------------------------------------------------------------
		
		/*
			Returns a vector of name of extensions.
			Asserts that every required extension is available.
		*/
		std::vector<const char *> GetExtensions();

		//Validation Layer
#ifdef NDEBUG
		bool CheckValidationLayerSupport() { return false; }
#else
		bool CheckValidationLayerSupport();
#endif
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info);
		static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayerDebugCallback	(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
																			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

		//Device Suitable
		bool IsPhysicalDeviceSuitable(const VkPhysicalDevice &device);
		QueueFamilyIndices FindQueueFamiliyIndices(const VkPhysicalDevice &device) const;
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice &device);

		SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice &deice) const;

		VkInstance m_VkInstance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkSurfaceKHR m_Surface;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE; //Implicitly destroyed with m_VkInstance
		VkDevice m_VkDevice;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		const std::vector<const char *> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		const std::vector<const char *> m_DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		bool m_RayTracingEnabled = true;

#ifdef NDEBUG
		bool m_EnableValidationLayer = false;
#else
		bool m_EnableValidationLayer = true;
#endif

	};

}
#endif
