#include "application.h"

#include <iostream>

int main() {
 
	gigno::Application *app = gigno::Application::MakeApp();

	int result = app->run();

	if (result) {
		printf("Gigno Application exited with code %d (not success).", result);
	}

	gigno::Application::ShutdownApp();

	return result;
}