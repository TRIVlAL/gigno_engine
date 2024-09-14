#include "application.h"

#include <iostream>

int main() {
 
	gigno::giApplication *app = gigno::giApplication::MakeApp();

	int result = app->run();

	if (result) {
		std::cout << "Gigno Application exited with code " << (int)result << " (not sccess)";
	}

	gigno::giApplication::ShutdownApp();

	return result;
}