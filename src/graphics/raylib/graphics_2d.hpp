#ifndef GALAXY_GRAPHICS_2D_H
#define GALAXY_GRAPHICS_2D_H

#include "graphics.hpp"
#include "../../orthtree.hpp"
#include "../../config.hpp"
#include "../../utils.hpp"


namespace graphics {
	class Graphics2D {
	private:
		float extent_x;
		float extent_y;

		float width;
		float height;
		float point_size;

		const config::Units& units;

		int win_context;

		/*bool use_video;
		video::Writer writer;*/

		template<typename TreePolicy>
		void draw_quadtree_node(const orthtree::TNode<typename TreePolicy::Item, 2, TreePolicy>* node) {
			float cx = node->bbox.center[0];
			float cy = node->bbox.center[1];
			float ex = node->bbox.extent[0];
			float ey = node->bbox.extent[1];

			float start_x = cx - ex + extent_x;
			float start_y = cy - ey + extent_y;

			raylib::DrawRectangleLinesEx(
				raylib::Rectangle {
					start_x*scale_x(), start_y*scale_y(),
					ex*2*scale_x(), ey*2*scale_y()
				},
				0.5,
				raylib::Color{50, 50, 100, 255}
			);

			if (!node->is_leaf()) {
				for (auto&& child : *(node->children)) {
					draw_quadtree_node(child.get());
				}
			} else {
				for (auto&& value : node->data) {
					typename TreePolicy::GetPoint get_point;
					auto point = get_point(value);

					raylib::DrawCircle(
						(point[0] + extent_x) * scale_x(),
						(point[1] + extent_y) * scale_y(),
						point_size/2.f,
						raylib::White
					);
				}
			}
		}

		template<typename TreePolicy>
		void draw_quadtree(const orthtree::QuadTree<typename TreePolicy::Item, TreePolicy>& qt) {
			draw_quadtree_node(&qt.root());
		}

		template<typename Scalar>
		void draw_graphics(Scalar time) {
			/* Draw timestamp */
			auto dist_unit = units.unit(config::Units::Quantity::DIST);
			auto time_unit = units.unit(config::Units::Quantity::TIME);

			auto time_text = formatf(time*time_unit.value, 0) + " " + time_unit.unit;

			raylib::DrawText(time_text.c_str(), 0, 0, 5, raylib::White);

			/* Draw scale */
			auto scale_text = formatf(dist_unit.value*10, 2) + " " + dist_unit.unit;
			std::size_t scale_length = 10 * scale_x();

			auto size = raylib::MeasureText(scale_text.c_str(), 5);

			raylib::DrawText(scale_text.c_str(), 0, 10, 5, raylib::White);

			std::size_t start_x = size/2-scale_length/2;
			std::size_t start_y = 25;
			std::size_t end_x = size/2-scale_length/2+scale_length;
			std::size_t end_y = start_y;

			raylib::DrawLine(start_x, start_y, end_x, end_y, raylib::White);
			raylib::DrawLine(start_x, start_y-3, start_x, start_y+3, raylib::White);
			raylib::DrawLine(end_x, start_y-3, end_x, start_y+3, raylib::White);
		}

	public:
		float scale_x() {
			return width/(extent_x*2.);
		}

		float scale_y() {
			return height/(extent_y*2.);
		}

		Graphics2D(config::Config cfg, const config::Units& units): units(units) {
			extent_x = cfg.get_or_fail<float>("simulation.size.extent.x");
			extent_y = cfg.get_or_fail<float>("simulation.size.extent.y");

			auto scale = cfg.get<float>("simulation.video.size.scale").value_or(1.);
			width = cfg.get<float>("simulation.video.size.width").value_or(extent_x*2.*scale);
			height = cfg.get<float>("simulation.video.size.height").value_or(extent_y*2.*scale);

			auto max_fps = cfg.get<std::size_t>("simulation.video.max_fps").value_or(30);

			/*use_video = cfg.get("simulation.video.output").has_value();
			if (use_video) {
				writer = video::Writer(cfg, width, height);
			}*/

			point_size = cfg.get_or_fail<float>("simulation.video.point_size");

			win_context = raylib::InitWindowPro(width, height, "galaxy", raylib::FLAG_WINDOW_RESIZABLE);
			raylib::SetActiveWindowContext(win_context);
			raylib::SetTargetFPS(max_fps);
		}

		~Graphics2D() {
			raylib::SetActiveWindowContext(win_context);
			raylib::CloseWindow();
		}

		template<typename Engine, typename TreePolicy>
		void show(typename TreePolicy::Item::Scalar time, const Engine* e, const orthtree::QuadTree<typename TreePolicy::Item, TreePolicy>& qt) {
			raylib::SetActiveWindowContext(win_context);
			raylib::BeginDrawing();

			raylib::ClearBackground(raylib::Black);

			draw_quadtree(qt);

			draw_graphics(time);

			raylib::EndDrawing();

			/*if (use_video) {
				writer.write(img);
			}*/
		}

		bool poll_close() {
			raylib::SetActiveWindowContext(win_context);
			return raylib::WindowShouldClose();
		}
	};
}

#endif