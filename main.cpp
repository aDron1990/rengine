#include "App.hpp"

#include <iostream>

int main() {
	try {
		auto app = App{ 800, 600, "Application" };
		app.run();
	}
	catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
}