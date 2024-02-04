#ifndef GALAXY_SIMPLE_GRAPHICS_3D_H
#define GALAXY_SIMPLE_GRAPHICS_3D_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/viz/viz3d.hpp>
#include <opencv2/viz/widgets.hpp>

#include "orthtree.hpp"
#include "config.hpp"
#include "utils.hpp"


namespace graphics {
	class Graphics3D {
	private:
		double extent_x;
		double extent_y;
		double extent_z;
		double scale;
		std::size_t plot_height;
		std::size_t plot_width;

		const config::Units& units;

		cv::viz::Viz3d viz;

	public:
		double output_width() {
			return extent_x*2*scale;
		}

		double output_height() {
			return extent_y*2*scale;
		}

		Graphics3D(config::Config cfg, const config::Units& units): units(units), viz(cv::viz::Viz3d("galaxy")) {
			extent_x = cfg.get_or_fail<double>("simulation.size.extent.x");
			extent_y = cfg.get_or_fail<double>("simulation.size.extent.y");
			extent_z = cfg.get_or_fail<double>("simulation.size.extent.z");
			scale = cfg.get_or_fail<double>("simulation.video.scale");

			viz.setWindowSize(cv::Size(output_height(), output_width()));
		}

		template<typename Scalar>
		void draw_graphics(Scalar time, cv::Mat& img) {
			/* Draw timestamp */
			auto dist_unit = units.unit(config::Units::Quantity::DIST);
			auto time_unit = units.unit(config::Units::Quantity::TIME);

			auto time_text = formatf(time*time_unit.value, 0) + " " + time_unit.unit;
			cv::putText(img,
				time_text,
				cv::Point(0, 15),
				cv::FONT_HERSHEY_DUPLEX,
				0.5,
				cv::Scalar(255, 255, 255),
				1
			);

			/* Draw scale */
			auto scale_text = formatf(dist_unit.value*10, 2) + " " + dist_unit.unit;
			std::size_t scale_length = 10 * scale;

			auto size = cv::getTextSize(scale_text, cv::FONT_HERSHEY_DUPLEX, 0.5, 1, nullptr);
			cv::putText(img,
				scale_text,
				cv::Point(0, 50),
				cv::FONT_HERSHEY_DUPLEX,
				0.5,
				cv::Scalar(255, 255, 255),
				1
			);

			cv::Point scale_start(size.width/2-scale_length/2, 30);
			cv::Point scale_end(size.width/2-scale_length/2+scale_length, scale_start.y);
			cv::line(img, scale_start, scale_end, cv::Scalar(255, 255, 255));
			cv::line(img, cv::Point(scale_start.x, scale_start.y-3), cv::Point(scale_start.x, scale_start.y+3), cv::Scalar(255, 255, 255));
			cv::line(img, cv::Point(scale_end.x, scale_end.y-3), cv::Point(scale_end.x, scale_end.y+3), cv::Scalar(255, 255, 255));
		}

		template<typename Engine, typename TreeType>
		void show(typename Engine::Scalar time, const Engine* e, const TreeType& tree) {
			viz.removeAllWidgets();

			cv::Mat img(output_height(), output_width(), CV_8UC3, cv::Scalar(0, 0, 0));
			draw_graphics(time, img);
			viz.setBackgroundTexture(img);

			std::vector<cv::Scalar> positions;
			for (auto&& body : e->bodies) {
				positions.emplace_back(body.pos[0], body.pos[1], body.pos[2]);
			}

			cv::Mat bodies(positions);
			auto cloud = cv::viz::WCloud(bodies);
			viz.showWidget("galaxy", cloud);

			viz.setBackgroundColor(cv::Scalar(0, 0, 0));

			/* Display */
			viz.spinOnce();
		}

		bool poll_close() {
			// Probably won't work:(
			return cv::pollKey() == 'x';
		}
	};
}

#endif