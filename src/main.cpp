#include "application.h"

#include <iostream>

int main() {
 
	gigno::Application *app = gigno::Application::MakeApp();

	int result = app->run();

	if (result) {
		std::cout << "Gigno Application exited with code " << (int)result << " (not sccess)";
	}

	gigno::Application::ShutdownApp();

	return result;
}