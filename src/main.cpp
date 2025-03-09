#include "application.h"
#include "debug/console/console.h"
#include "debug/console/convar.h"

namespace gigno {
	CONVAR(bool, should_restart, false, "If true, the app will auto restart when closing.");
}

int main() {
	int result = 0;
 
	do {
		{
			gigno::Application app{};
			result = app.run();
		}

		gigno::Console::LogInfo("Gigno Engine exited with code %d.", result);

	} while((bool)gigno::convar_should_restart);

	return result;
}