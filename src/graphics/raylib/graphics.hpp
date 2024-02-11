#ifndef GALAXY_GRAPHICS_H
#define GALAXY_GRAPHICS_H


namespace raylib {
	extern "C" {
		#include <raylib.h>
		#include <rcamera.h>
	}

	static constexpr raylib::Color White = raylib::Color{255, 255, 255, 255};
	static constexpr raylib::Color Black = raylib::Color{0, 0, 0, 255};
}


#endif
