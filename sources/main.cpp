#include "App.hpp"

#include <iostream>

int main() {
	try {
		auto app = App{ 1600, 900, "Application" };
		app.run();
	}
	catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
}