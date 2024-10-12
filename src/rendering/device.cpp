#include <GLFW/glfw3.h>
#include "../core_macros.h"
#include <cstring>
#include <set>
#include "swapchain.h"
#include "device.h"

namespace gigno {

	Device::Device(const Window *window) {
		CreateInstance();
		if (m_EnableValidationLayer) {
			CreateDebugMessenger();
		}
		CreateSurface(window);
		PickPhysicalDevice();
		CreateVulkanDevice();
	}


	Device::~Device() {
		vkDestroyDevice(m_VkDevice, nullptr);

		//Destroy Debug Messenger
		if (m_EnableValidationLayer) {
			auto funcDestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_VkInstance, "vkDestroyDebugUtilsMessengerEXT");
			if (funcDestroyDebugMessenger != nullptr) {
				funcDestroyDebugMessenger(m_VkInstance, m_DebugMessenger, nullptr);
			}
		}

		vkDestroySurfaceKHR(m_VkInstance, m_Surface, nullptr);
		vkDestroyInstance(m_VkInstance, nullptr);
	}

	

	void Device::CreateInstance() {
		if (m_EnableValidationLayer && !CheckValidationLayerSupport()) {
			std::cerr << "ERROR ! VULKAN Validation layer requested, but not avaliable !\n";
		}

		VkApplicationInfo appinfo;
		appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appinfo.pNext = nullptr;
		appinfo.pApplicationName = "Gigno Engine App";
		appinfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 0);
		appinfo.pEngineName = "Gigno Engine";
		appinfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 0);
		appinfo.apiVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);

		std::vector<const char *> extensions = GetRequiredExtensions();

		VkInstanceCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createinfo.pApplicationInfo = &appinfo;
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
		if (m_EnableValidationLayer) {
			createinfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			createinfo.ppEnabledLayerNames = m_ValidationLayers.data();

			PopulateDebugMessengerCreateInfo(debugMessengerCreateInfo);

			createinfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugMessengerCreateInfo;
		}
		else {
			createinfo.enabledLayerCount = 0;
			createinfo.pNext = nullptr;
		}
		createinfo.enabledLayerCount = 0;
		createinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createinfo.ppEnabledExtensionNames = extensions.data();
		

		VkResult result = vkCreateInstance(&createinfo, nullptr, &m_VkInstance);

		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Instance ! Vulkan error code : " << (int)result);
		}
	}

	void Device::CreateDebugMessenger() {
		if (!m_EnableValidationLayer) return;

		VkDebugUtilsMessengerCreateInfoEXT createinfo;
		PopulateDebugMessengerCreateInfo(createinfo);

		auto funcCreateMessenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_VkInstance, "vkCreateDebugUtilsMessengerEXT");
		VkResult result;
		if (funcCreateMessenger != nullptr) {
			result = funcCreateMessenger(m_VkInstance, &createinfo, nullptr, &m_DebugMessenger);
		}
		else {
			result = VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		if (result != VK_SUCCESS) {
			ERR_MSG("VALIDATION LAYER : Failed to create Debug Messenger ! Vulkan error code " << (int)result);
		}
	}

	void Device::PickPhysicalDevice() {

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);

		ASSERT_MSG(deviceCount > 0, "No GPU with Vulkan Support found !");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

		for (auto &device : devices) {
			if (IsPhysicalDeviceSuitable(device)) {
				m_PhysicalDevice = device;
				break;
			}
		}

		ASSERT_MSG((m_PhysicalDevice != VK_NULL_HANDLE), "No Suitable GPU Found !");
	}

	void Device::CreateVulkanDevice() {
		QueueFamilyIndices indices = FindQueueFamiliyIndices(m_PhysicalDevice);
		
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilyIndices = { indices.graphicFamily.value(), indices.presentFamily.value()};

		float queuePrioriy = 1.0f;

		for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndices) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.queueFamilyIndex = indices.graphicFamily.value();
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePrioriy;
			queueCreateInfos.push_back(queueCreateInfo);
		}
		
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		if (m_EnableValidationLayer) {
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_VkDevice);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Vulkan Logical Device ! Vulkan error code : " << (int)result);
		}

		vkGetDeviceQueue(m_VkDevice, indices.graphicFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_VkDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
	}

	void Device::CreateSurface(const Window *window) {
		window->CreateWindowSurface(m_VkInstance, &m_Surface);
	}

#ifndef NDEBUG
	bool Device::CheckValidationLayerSupport() {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> avaliables(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, avaliables.data());

		for (const char *layer : m_ValidationLayers) {
			bool found = false;

			for (VkLayerProperties& avaliable : avaliables) {
				if (strcmp(layer, avaliable.layerName) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				return false;
			}
		}
		return true;
	}
#endif

	VKAPI_ATTR VkBool32 VKAPI_CALL Device::ValidationLayerDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData) {
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
			std::cout << "VERBOSE: VULKAN Validation Layer : " << pCallbackData->pMessage << std::endl;
		}
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			std::cerr << "ERROR: VULKAN Validation Layer : " << pCallbackData->pMessage << std::endl;
		}
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			std::cout << "WARNING: VULKAN Validation Layer : " << pCallbackData->pMessage << std::endl;
		}
		else {
			std::cout << "VULKAN Validation Layer : " << pCallbackData->pMessage << std::endl;
		}
		return VK_FALSE;
	}

	std::vector<const char *> Device::GetRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_EnableValidationLayer) {
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void Device::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &info) {
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.pNext = nullptr;
		info.messageSeverity =
#if VERBOSE
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
#endif
#if LOG_VULKAN_INFOS
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
#endif
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
		info.pfnUserCallback = ValidationLayerDebugCallback;
		info.pUserData = nullptr;
	}

	bool Device::IsPhysicalDeviceSuitable(const VkPhysicalDevice& device) {
		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.presentModes.empty() && !swapChainSupport.formats.empty();
		}

		return FindQueueFamiliyIndices(device).IsComplete() && extensionsSupported && swapChainAdequate;
	}

	QueueFamilyIndices Device::FindQueueFamiliyIndices(const VkPhysicalDevice &device) const {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

		QueueFamilyIndices indices{};
		int i = 0;
		for (const auto &queueFamilyProperty : queueFamilyProperties) {
			VkBool32 supportsPresent = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &supportsPresent);
			if (supportsPresent) {
				indices.presentFamily = i;
			}
			if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicFamily = i;
			}

			if (indices.IsComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	bool Device::CheckDeviceExtensionSupport(const VkPhysicalDevice &device) {
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensionPorperties(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensionPorperties.data());

		std::set<std::string> uniqueRequiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());
		for (auto &extentionProperty : extensionPorperties) {
			uniqueRequiredExtensions.erase(extentionProperty.extensionName);
		}

		return uniqueRequiredExtensions.empty();
	}

	SwapChainSupportDetails Device::QuerySwapChainSupport(const VkPhysicalDevice &device) const {
		SwapChainSupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.surfaceCapabilities);

		uint32_t formatsCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatsCount, nullptr);

		if (formatsCount != 0) { 
				details.formats.resize(formatsCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatsCount, details.formats.data());
		}

		uint32_t presentsCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentsCount, nullptr);

		if (presentsCount != 0) {
			details.presentModes.resize(presentsCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &formatsCount, details.presentModes.data());
		}

		return details;
	}
}