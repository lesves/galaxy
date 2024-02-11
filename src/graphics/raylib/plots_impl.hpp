#ifndef GALAXY_PLOTS_IMPL_H
#define GALAXY_PLOTS_IMPL_H

#include "graphics.hpp"


namespace plots {
	raylib::Color color(unsigned char r, unsigned char g, unsigned char b) {
		return raylib::Color { r, g, b, 255 };
	}

	class PlotWindow {
	private:
		int win_context;

		void activate() {
			raylib::SetActiveWindowContext(win_context);
		}

	public:
		PlotWindow(std::size_t w, std::size_t h) {
			win_context = raylib::InitWindowPro(w, h, "plot", 0);
			activate();
		}

		void set_name(const std::string& name) {
			activate();
			raylib::SetWindowTitle(name.c_str());
		}

		void begin_plot() {
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