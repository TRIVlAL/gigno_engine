#include <GLFW/glfw3.h>
#include "../error_macros.h"
#include <cstring>
#include <set>
#include "device.h"
#include "../application.h"

namespace gigno {
	
	void Device::Init(const Window *window) {

		CreateInstance();

		if (m_EnableValidationLayer) {
			CreateDebugMessenger();
		}

		window->CreateWindowSurface(m_VkInstance, &m_Surface);
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
			Console::LogError("Device::CreateInstance : ERROR ! VULKAN Validation layer requested, but not avaliable !");
		}

		VkApplicationInfo appinfo;
		appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appinfo.pNext = nullptr;
		appinfo.pApplicationName = "Gigno Engine Physics Demo";
		appinfo.applicationVersion = VK_MAKE_API_VERSION(0, 4, 0, 0);
		appinfo.pEngineName = "Gigno Engine";
		// Note for later : DO NOT CHANGE THE API VERSION UNLESS U SURE WHAT U DOING (More than myself anyway)
		appinfo.apiVersion = VK_MAKE_API_VERSION(1, 0, 0, 0); 
		appinfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 0);
		
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

		const std::vector<const char *> extensions = GetExtensions();

		createinfo.enabledLayerCount = 0;
		createinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createinfo.ppEnabledExtensionNames = extensions.data();
		
		VULKAN_CHECK(vkCreateInstance(&createinfo, nullptr, &m_VkInstance), "Failed to create Vulkan Instance ! ");
	}

	void Device::CreateDebugMessenger() {
		if (!m_EnableValidationLayer) return;

		VkDebugUtilsMessengerCreateInfoEXT createinfo;
		PopulateDebugMessengerCreateInfo(createinfo);

		auto func_create_messenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_VkInstance, "vkCreateDebugUtilsMessengerEXT");
		VkResult result;
		if (func_create_messenger) {
			result = func_create_messenger(m_VkInstance, &createinfo, nullptr, &m_DebugMessenger);
		}
		else {
			Console::LogWarning("VULKAN DebugUtilsMessengerEX Debug Messenger Extension is not present !");
			result = VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		if(result != VK_SUCCESS) {
			Console::LogWarning("VULKANE Failed to create Debug Messenger ! Vulkan error code : %d", (int)result);
			m_EnableValidationLayer = false;
		}

	}

	void Device::PickPhysicalDevice() {

		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(m_VkInstance, &device_count, nullptr);

		if(device_count == 0) {
			Console::LogError("No GPU with Vulkan Support Found !");
			Application::Singleton()->SetExit(EXIT_FAILED_VULKAN_SUPPORT);
		}
		
		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(m_VkInstance, &device_count, devices.data());
		
		for (auto &device : devices) {
			if (IsPhysicalDeviceSuitable(device)) {
				m_PhysicalDevice = device;
				break;
			}
		}
		
		if(m_PhysicalDevice == VK_NULL_HANDLE) {
			Console::LogError("No Suitable GPU Found !");
			Application::Singleton()->SetExit(EXIT_FAILED_RENDERER);
		}
	}

	void Device::CreateVulkanDevice() {
		QueueFamilyIndices indices = FindQueueFamiliyIndices(m_PhysicalDevice);
		
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queue_family_indices = { indices.graphicFamily.value(), indices.presentFamily.value()};
		queue_create_infos.reserve(unique_queue_family_indices.size());

		float queue_prioriy = 1.0f;

		for (uint32_t queueFamilyIndex : unique_queue_family_indices) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.queueFamilyIndex = indices.graphicFamily.value();
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queue_prioriy;
			queue_create_infos.push_back(queueCreateInfo);
		}
		
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.queueCreateInfoCount = queue_create_infos.size();
		deviceCreateInfo.pQueueCreateInfos = queue_create_infos.data();
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

		VULKAN_CHECK(vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_VkDevice), "Failed to create Vulkan Logical Device ! ");

		vkGetDeviceQueue(m_VkDevice, indices.graphicFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_VkDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
	}

#ifndef NDEBUG
	bool Device::CheckValidationLayerSupport() {
		uint32_t layer_count = 0;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<VkLayerProperties> avaliables(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, avaliables.data());

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
			VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
			VkDebugUtilsMessageTypeFlagsEXT message_type,
			const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
			void *pUserData) {
		if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
			printf("VERBOSE: VULKAN Validation Layer : %s\n", callback_data->pMessage);
		}
		else if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			Console::LogError("VULKAN Validation Layer : %s", callback_data->pMessage);
		}
		else if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			Console::LogWarning ("VULKAN Validation Layer WARNING : %s\n", callback_data->pMessage);
		}
		else {
			Console::LogInfo ("VULKAN Validation Layer : %s\n", callback_data->pMessage);
		}
		return VK_FALSE;
	}
	
	std::vector<const char *> Device::GetExtensions() {
		uint32_t avaliable_extensions_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &avaliable_extensions_count, nullptr);
		std::vector<VkExtensionProperties> avaliable_extensions{avaliable_extensions_count};
		vkEnumerateInstanceExtensionProperties(nullptr, &avaliable_extensions_count, avaliable_extensions.data());
		
		//Get glfw required extensions
		uint32_t glfw_extension_count = 0;
		const char **glfw_extensions;
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		std::vector<const char *> required_extensions{glfw_extensions, glfw_extensions + glfw_extension_count};
		
		//Get Validation Layer Required extension
		if (m_EnableValidationLayer) {
			required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		
		//Check if required extensions are avaliable
		for(const char *req : required_extensions) {
			bool found = false;
			for (VkExtensionProperties avail : avaliable_extensions) {
				if(strcmp(req, avail.extensionName) == 0) {
					found = true;
					break;
				}
			}
			ASSERT_MSG_V((found), required_extensions, "VULKAN : Requires extension %s but it is not avaliable !", req);
		}

		std::vector<const char *> all_extensions = required_extensions;
		
		return all_extensions;
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
		bool extensions_supported = CheckDeviceExtensionSupport(device);

		bool swap_chain_adequate = false;
		if (extensions_supported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swap_chain_adequate = !swapChainSupport.presentModes.empty() && !swapChainSupport.formats.empty();
		}

		return FindQueueFamiliyIndices(device).IsComplete() && extensions_supported && swap_chain_adequate;
	}

	QueueFamilyIndices Device::FindQueueFamiliyIndices(const VkPhysicalDevice &device) const {
		uint32_t queue_fam_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_fam_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_fam_properties(queue_fam_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_fam_count, queue_fam_properties.data());

		QueueFamilyIndices indices{};

		for (size_t i = 0; i < queue_fam_properties.size(); i++) {
			const auto &queue_fam_property = queue_fam_properties[i];
			VkBool32 supports_present = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &supports_present);
			if (supports_present) {
				indices.presentFamily = i;
			}
			if (queue_fam_property.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicFamily = i;
			}

			if (indices.IsComplete()) {
				break;
			}
		}

		return indices;
	}

	bool Device::CheckDeviceExtensionSupport(const VkPhysicalDevice &device) {
		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> extension_porperties(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extension_porperties.data());

		std::set<std::string> unique_required_extensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());
		for (auto &extentionProperty : extension_porperties) {
			unique_required_extensions.erase(extentionProperty.extensionName);
		}

		return unique_required_extensions.empty();
	}

	SwapChainSupportDetails Device::QuerySwapChainSupport(const VkPhysicalDevice &device) const {
		SwapChainSupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.surfaceCapabilities);

		uint32_t formats_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formats_count, nullptr);

		if (formats_count != 0) { 
			details.formats.resize(formats_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formats_count, details.formats.data());
		}

		uint32_t presents_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presents_count, nullptr);

		if (presents_count != 0) {
			details.presentModes.resize(presents_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &formats_count, details.presentModes.data());
		}

		return details;
	}
}