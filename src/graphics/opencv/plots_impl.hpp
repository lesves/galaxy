#ifndef GALAXY_PLOTS_IMPL_H
#define GALAXY_PLOTS_IMPL_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace plots {
	cv::Scalar color(unsigned char r, unsigned char g, unsigned char b) {
		return cv::Scalar(r, g, b);
	}

	class PlotWindow {
	private:
		cv::Mat plot_img;
		std::string name_;

	public:
		PlotWindow(std::size_t w, std::size_t h): name_("plot"), plot_img(h, w, CV_8UC3, cv::Scalar(0, 0, 0)) {}

		void set_name(const std::string& name) {
			name_ = name;
		}

		void begin_plot() {
			plot_img = cv::Scalar(0, 0, 0);
		}

		void line(double start_x, double start_y, double end_x, double end_y, cv::Scalar color) {
			cv::line(plot_img, cv::Point(start_x, start_y), cv::Point(end_x, end_y), color);
		}

		void end_plot() {
			cv::imshow(name_, plot_img);
		}
	};
}

#endif