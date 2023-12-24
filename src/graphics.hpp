#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "orthtree.hpp"

namespace graphics2d {
	template<typename P>
	void draw_quadtree_node(cv::Mat& img, const P& policy, const orthtree::Node<double, 2,  P>* node) {
		auto cx = node->bbox.center[0];
		auto cy = node->bbox.center[1];
		auto ex = node->bbox.extent[0];
		auto ey = node->bbox.extent[1];

		auto start_x = cx - ex + policy.extent_x;
		auto start_y = cy - ey + policy.extent_y;
		auto end_x = cx + ex + policy.extent_x;
		auto end_y = cy + ey + policy.extent_y;

		cv::rectangle(
			img, 
			cv::Point(start_x*policy.display_scale, start_y*policy.display_scale), 
			cv::Point(end_x*policy.display_scale, end_y*policy.display_scale), 
			cv::Scalar(255, 255, 255), 
			1
		);

		if (node->children.has_value()) {
			for (auto&& child : *(node->children)) {
				draw_quadtree_node(img, policy, child.get());
			}
		} else {
			for (auto&& point : node->points) {
				cv::circle(
					img, 
					cv::Point(
						(point[0] + policy.extent_x) * policy.display_scale, 
						(point[1] + policy.extent_y) * policy.display_scale
					), 
					3, 
					cv::Scalar(255, 255, 255), 
					-1
				);
			}
		}
	}

	template<typename P>
	void draw_quadtree(cv::Mat& img, const P& policy, const orthtree::QuadTree<double, P>& qt) {
		draw_quadtree_node(img, policy, &qt.root());
	}

	template<typename P>
	void draw_graphics(cv::Mat& img, const P& policy) {
		/* Draw timestamp */
		auto time_text = "0 mil. let";
		cv::putText(img,
			time_text,
			cv::Point(0, 15),
			cv::FONT_HERSHEY_DUPLEX,
			0.5,
			cv::Scalar(255, 255, 255),
			1
		);

		/* Draw scale */
		auto scale_text = "1" + policy.shown_unit_name;
		auto scale_length = (std::size_t)(1/policy.scale) * policy.display_scale;

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
	}

	template<typename P>
	void show(const P& policy, const orthtree::QuadTree<double, P>& qt) {
		auto out_width = policy.width*policy.display_scale;
		auto out_height = policy.height*policy.display_scale;

		cv::Mat img(out_width, out_height, CV_8UC3, cv::Scalar(0, 0, 0));
		draw_quadtree(img, policy, qt);

		draw_graphics(img, policy);

		/* Display */
		cv::imshow("galaxy", img);
		cv::waitKey(); 
	}
}