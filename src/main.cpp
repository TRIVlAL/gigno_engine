#include "application.h"
#include "debug/console/console.h"

int main() {
 
	gigno::Application *app = gigno::Application::MakeApp();

	int result = app->run();

	gigno::Application::ShutdownApp();

	gigno::Console::LogInfo("Gigno Engine exited with code %d.", result);

	return result;
}