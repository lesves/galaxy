#ifndef GALAXY_SIMPLE_GRAPHICS_3D_H
#define GALAXY_SIMPLE_GRAPHICS_3D_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/viz/viz3d.hpp>
#include <opencv2/viz/types.hpp>
#include <opencv2/viz/widgets.hpp>

#include <algorithm>

#include "orthtree.hpp"
#include "config.hpp"
#include "utils.hpp"


namespace graphics {
	class Graphics3D {
	private:
		double extent_x;
		double extent_y;
		double extent_z;

		double height;
		double width;

		double point_size;

		bool show_bbox;

		const config::Units& units;

		cv::viz::Viz3d viz;

	public:
		Graphics3D(config::Config cfg, const config::Units& units): units(units), viz(cv::viz::Viz3d("galaxy")) {
			extent_x = cfg.get_or_fail<double>("simulation.size.extent.x");
			extent_y = cfg.get_or_fail<double>("simulation.size.extent.y");
			extent_z = cfg.get_or_fail<double>("simulation.size.extent.z");

			point_size = cfg.get_or_fail<double>("simulation.video.point_size");

			show_bbox = cfg.get<bool>("simulation.video.show_bbox").value_or(true);
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

			if (show_bbox) {
				cv::Point3d text_x_pos(0, -extent_y, -extent_z);
				viz.showWidget("text_x", cv::viz::WText3D(formatf(dist_unit.value*extent_x*2, 2) + dist_unit.unit, text_x_pos, std::max(1., extent_x/50), true));
				cv::Point3d text_y_pos(-extent_x, 0, -extent_z);
				viz.showWidget("text_y", cv::viz::WText3D(formatf(dist_unit.value*extent_y*2, 2) + dist_unit.unit, text_y_pos, std::max(1., extent_y/50), true));
				cv::Point3d text_z_pos(-extent_x, -extent_y, 0);
				viz.showWidget("text_z", cv::viz::WText3D(formatf(dist_unit.value*extent_z*2, 2) + dist_unit.unit, text_z_pos, std::max(1., extent_z/50), true));

				cv::Point3d maxp(extent_x, extent_y, extent_z);
				cv::Point3d minp(-extent_x, -extent_y, -extent_z);
				viz.showWidget("bbox", cv::viz::WCube(minp, maxp));

				if (time == 0.) {
					// Fix for viz bug with bad zoom with texts
					viz.resetCamera();
				}
			}
		}

		template<typename Engine, typename TreeType>
		void show(typename Engine::Scalar time, const Engine* e, const TreeType& tree) {
			viz.removeAllWidgets();

			auto size = viz.getWindowSize();
			cv::Mat img(size.height, size.width, CV_8UC3, cv::Scalar(0, 0, 0));
			draw_graphics(time, img);
			viz.setBackgroundTexture(img);

			std::vector<cv::Scalar> positions;
			for (auto&& body : e->bodies) {
				positions.emplace_back(body.pos[0], body.pos[1], body.pos[2]);
			}

			cv::Mat bodies(positions);
			auto cloud = cv::viz::WCloud(bodies);
			viz.showWidget("galaxy", cloud);
			viz.setRenderingProperty("galaxy", cv::viz::POINT_SIZE, point_size);

			viz.setBackgroundColor(cv::Scalar(0, 0, 0));

			/* Display */
			viz.spinOnce();
		}

		bool poll_close() {
			// Probably won't work:(
			return cv::pollKey() == 'x' || viz.wasStopped();
		}
	};
}

#endif