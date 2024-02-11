#ifndef GALAXY_PLOTS_IMPL_H
#define GALAXY_PLOTS_IMPL_H

#include "graphics.hpp"


namespace plots {
	raylib::Color color(unsigned char r, unsigned char g, unsigned char b) {
		return raylib::Color { r, g, b, 255 };
	}

	class PlotWindow {
	private:
		bool is_running;
		int win_context;
		std::size_t width;
		std::size_t height;

		void activate() {
			raylib::SetActiveWindowContext(win_context);
		}

	public:
		PlotWindow(std::size_t w, std::size_t h): width(w), height(h), is_running(false) {}

		void set_name(const std::string& name) {
			activate();
			raylib::SetWindowTitle(name.c_str());
		}

		void begin_plot() {
			if (!is_running) {
				win_context = raylib::InitWindowPro(width, height, "plot", 0);
				is_running = true;
			}

			activate();
			raylib::BeginDrawing();
		}

		void line(double start_x, double start_y, double end_x, double end_y, raylib::Color color) {
			raylib::DrawLine(start_x, start_y, end_x, end_y, color);
		}

		void end_plot() {
			raylib::EndDrawing();
		}
	};
}

#endif