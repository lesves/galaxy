#ifndef GALAXY_TREE_GRAPHICS_2D_H
#define GALAXY_TREE_GRAPHICS_2D_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../../orthtree.hpp"
#include "../../config.hpp"
#include "../../video.hpp"
#include "../../utils.hpp"


namespace graphics {
	class Graphics2D {
	private:
		double extent_x;
		double extent_y;

		double width;
		double height;
		double point_size;

		const config::Units& units;

		bool use_video;
		video::Writer writer;

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

			cv::rectangle(
				img, 
				cv::Point(start_x*scale_x(), start_y*scale_y()), 
				cv::Point(end_x*scale_x(), end_y*scale_y()), 
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
							(point[0] + extent_x) * scale_x(), 
							(point[1] + extent_y) * scale_y()
						), 
						point_size, 
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
			std::size_t scale_length = 10 * scale_x();

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

	public:
		double scale_x() {
			return width/(extent_x*2.);
		}

		double scale_y() {
			return height/(extent_y*2.);
		}

		Graphics2D(config::Config cfg, const config::Units& units): units(units) {
			extent_x = cfg.get_or_fail<double>("simulation.size.extent.x");
			extent_y = cfg.get_or_fail<double>("simulation.size.extent.y");

			auto scale = cfg.get<double>("simulation.video.size.scale").value_or(1.);
			width = cfg.get<double>("simulation.video.size.width").value_or(extent_x*2.*scale);
			height = cfg.get<double>("simulation.video.size.height").value_or(extent_y*2.*scale);

			use_video = cfg.get("simulation.video.output").has_value();
			if (use_video) {
				writer = video::Writer(cfg, width, height);
			}

			point_size = cfg.get_or_fail<double>("simulation.video.point_size");

			cv::namedWindow("galaxy", cv::WINDOW_NORMAL);
		}

		~Graphics2D() {
			cv::destroyWindow("galaxy");
		}

		template<typename Engine, typename TreePolicy>
		void show(typename TreePolicy::Item::Scalar time, const Engine* e, const orthtree::QuadTree<typename TreePolicy::Item, TreePolicy>& qt) {
			cv::Mat img(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
			draw_quadtree(img, qt);

			draw_graphics(time, img);

			/* Display */
			cv::imshow("galaxy", img);

			if (time == 0.) {
				cv::resizeWindow("galaxy", width, height);
			}

			if (use_video) {
				writer.write(img);
			}
		}

		bool poll_close() {
			return cv::pollKey() == 'x';
		}
	};
}

#endif