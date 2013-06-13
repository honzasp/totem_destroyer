#include <stdexcept>
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <guichan.hpp>
#include "json/exceptions.hpp"
#include "menu.hpp"

int main(int argc, char **argv)
{
	try {
		Menu menu;
		menu.run();
	} catch(std::runtime_error e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	} catch(gcn::Exception e) {
		std::cerr << "Guichan error: " << e.getMessage() << std::endl;
		return 1;
	} catch(std::exception e) {
		std::cerr << "Std exception: " << e.what() << std::endl;
		return 1;
	} catch(json::Exception e) {
		std::cerr << "JSON exception in main! Msg: " << e.what() << std::endl;
		return 1;
	} catch(...) {
		std::cerr << "Unknown exception" << std::endl;
		return 1;
	}
}
