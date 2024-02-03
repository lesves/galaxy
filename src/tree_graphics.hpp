#ifndef GALAXY_TREE_GRAPHICS_H
#define GALAXY_TREE_GRAPHICS_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "orthtree.hpp"
#include "config.hpp"
#include "utils.hpp"


namespace tree_graphics {
	class Graphics2D {
	private:
		double extent_x;
		double extent_y;
		double scale;
		std::size_t plot_height;
		std::size_t plot_width;

		const config::Units& units;

	public:
		double output_width() {
			return extent_x*2*scale;
		}

		double output_height() {
			return extent_y*2*scale;
		}

		Graphics2D(config::Config cfg, const config::Units& units): units(units) {
			extent_x = cfg.get_or_fail<double>("simulation.size.extent.x");
			extent_y = cfg.get_or_fail<double>("simulation.size.extent.x");
			scale = cfg.get_or_fail<double>("simulation.video.scale");

			plot_height = cfg.get_or_fail<std::size_t>("simulation.plots.energy.size.height");
			plot_width = cfg.get_or_fail<std::size_t>("simulation.plots.energy.size.width");
		}

		template<typename TreePolicy>
		void draw_quadtree_node(cv::Mat& img, const orthtree::TNode<typename TreePolicy::Item, 2, TreePolicy>* node) {
			auto cx = node->bbox.center[0];
			auto cy = node->bbox.center[1];
			auto ex = node->bbox.extent[0];
			auto ey = node->bbox.extent[1];

			auto start_x = cx - ex + extent_x;
			auto start_y = cy - ey + extent_y;
			auto end_x = cx + ex + extent_x;
			auto end_y = cy + ey + extent_y;

			//std::cout << "nc: " << cx << " " << cy << " cnt: " << node->accum_value.count << "\n";
			cv::rectangle(
				img, 
				cv::Point(start_x*scale, start_y*scale), 
				cv::Point(end_x*scale, end_y*scale), 
				cv::Scalar(100, 50, 50), 
				1
			);

			if (!node->is_leaf()) {
				for (auto&& child : *(node->children)) {
					draw_quadtree_node(img, child.get());
				}
			} else {
				for (auto&& value : node->data) {
					typename TreePolicy::GetPoint get_point;
					auto point = get_point(value);

					cv::circle(
						img, 
						cv::Point(
							(point[0] + extent_x) * scale, 
							(point[1] + extent_y) * scale
						), 
						2, 
						cv::Scalar(255, 255, 255), 
						-1
					);
				}
			}
		}

		template<typename TreePolicy>
		void draw_quadtree(cv::Mat& img, const orthtree::QuadTree<typename TreePolicy::Item, TreePolicy>& qt) {
			draw_quadtree_node(img, &qt.root());
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

		template<typename TreePolicy>
		void show(typename TreePolicy::Item::Scalar time, const orthtree::QuadTree<typename TreePolicy::Item, TreePolicy>& qt) {
			cv::Mat img(output_height(), output_width(), CV_8UC3, cv::Scalar(0, 0, 0));
			draw_quadtree(img, qt);

			draw_graphics(time, img);

			/* Display */
			cv::imshow("galaxy", img);
		}

		bool poll_close() {
			return cv::pollKey() == 'x';
		}
	};
}

#endif