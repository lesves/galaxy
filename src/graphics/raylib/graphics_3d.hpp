#ifndef GALAXY_GRAPHICS_3D_H
#define GALAXY_GRAPHICS_3D_H

#include "graphics.hpp"
#include "../../orthtree.hpp"
#include "../../config.hpp"
#include "../../utils.hpp"

namespace graphics {
	class Graphics3D {
	private:
		float extent_x;
		float extent_y;
		float extent_z;

		float width;
		float height;

		float point_size;
		bool show_bbox;

		const config::Units& units;

		raylib::Camera camera;

		int win_context;

		static constexpr float far_divisor = 100.f;
		static constexpr float sensitivity = 0.002f;

		/*bool use_video;
		video::Writer writer;*/

		template<typename Engine>
		void show_frame(typename Engine::Scalar time, const Engine* e) {
			raylib::BeginDrawing();
				raylib::ClearBackground(raylib::Black);

				raylib::BeginMode3D(camera);
					raylib::DrawCubeWires(raylib::Vector3{0, 0, 0}, extent_x*2/far_divisor, extent_y*2/far_divisor, extent_z*2/far_divisor, raylib::White);

					for (auto&& body : e->bodies) {
						auto center = raylib::Vector3{
							static_cast<float>(body.pos[0])/far_divisor, 
							static_cast<float>(body.pos[1])/far_divisor, 
							static_cast<float>(body.pos[2])/far_divisor
						};
						DrawSphere(center, point_size/10.f/far_divisor, raylib::White);
					}

				raylib::EndMode3D();

				draw_graphics(time);
			raylib::EndDrawing();

			/*if (use_video) {
				writer.write(img);
			}*/
		}

		bool btn_pressed() {
			return raylib::IsMouseButtonDown(raylib::MOUSE_BUTTON_LEFT) || \
				raylib::IsMouseButtonPressed(raylib::MOUSE_BUTTON_LEFT) || \
				raylib::IsMouseButtonDown(raylib::MOUSE_BUTTON_MIDDLE) || \
				raylib::IsMouseButtonPressed(raylib::MOUSE_BUTTON_MIDDLE);
		}

		template<typename Scalar>
		void draw_graphics(Scalar time) {
			/* Draw timestamp */
			auto dist_unit = units.unit(config::Units::Quantity::DIST);
			auto time_unit = units.unit(config::Units::Quantity::TIME);

			auto time_text = formatf(time*time_unit.value, 0) + " " + time_unit.unit;

			raylib::DrawText(time_text.c_str(), 0, 0, 5, raylib::White);

			/* Draw scale */
			/*
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
			*/
		}

	public:
		float scale_x() {
			return width/(extent_x*2.);
		}

		float scale_y() {
			return height/(extent_y*2.);
		}

		Graphics3D(config::Config cfg, const config::Units& units): units(units) {
			extent_x = cfg.get_or_fail<float>("simulation.size.extent.x");
			extent_y = cfg.get_or_fail<float>("simulation.size.extent.y");
			extent_z = cfg.get_or_fail<float>("simulation.size.extent.z");

			auto scale = cfg.get<float>("simulation.video.size.scale").value_or(1.);
			width = cfg.get<float>("simulation.video.size.width").value_or(extent_x*2.*scale);
			height = cfg.get<float>("simulation.video.size.height").value_or(extent_y*2.*scale);

			show_bbox = cfg.get<bool>("simulation.video.show_bbox").value_or(true);

			auto max_fps = cfg.get<std::size_t>("simulation.video.max_fps").value_or(30);

			/*use_video = cfg.get("simulation.video.output").has_value();
			if (use_video) {
				writer = video::Writer(cfg, width, height);
			}*/

			point_size = cfg.get_or_fail<float>("simulation.video.point_size");

			win_context = raylib::InitWindowPro(width, height, "galaxy", raylib::FLAG_WINDOW_RESIZABLE);
			raylib::SetActiveWindowContext(win_context);
			raylib::SetTargetFPS(max_fps);

			camera = { 0 };
			camera.position = raylib::Vector3 { 0.0f, 0.0f, -4*std::max(std::max(extent_x, extent_y), extent_z)/far_divisor };
			camera.target = raylib::Vector3 { 0.0f, 0.0f, 0.0f };
			camera.up = raylib::Vector3 { 0.0f, 10.0f, 0.0f };
			camera.fovy = 45.0f;
			camera.projection = raylib::CAMERA_PERSPECTIVE;
		}

		~Graphics3D() {
			raylib::SetActiveWindowContext(win_context);
			raylib::CloseWindow();
		}

		template<typename Engine, typename TreeType>
		void show(typename Engine::Scalar time, const Engine* e, const TreeType& t) {
			raylib::SetActiveWindowContext(win_context);
			show_frame(time, e);

			while (btn_pressed() && !raylib::WindowShouldClose()) {
				show_frame(time, e);

				auto delta = raylib::GetMouseDelta();
				raylib::Vector3 move = {0};
				if (raylib::IsMouseButtonDown(raylib::MOUSE_BUTTON_MIDDLE)) {
					move.x = -delta.x*sensitivity;
					move.y = -delta.y*sensitivity;
				} else {
					raylib::CameraYaw(&camera, -delta.x*sensitivity, true);
					raylib::CameraPitch(&camera, -delta.y*sensitivity, true, true, false);
				}

				auto wheel = raylib::GetMouseWheelMove();
				raylib::UpdateCameraPro(&camera, raylib::Vector3{0}, move, wheel);
			}

			auto wheel = raylib::GetMouseWheelMove();
			raylib::UpdateCameraPro(&camera, raylib::Vector3{0}, raylib::Vector3{0}, wheel);
		}

		bool poll_close() {
			raylib::SetActiveWindowContext(win_context);
			return raylib::WindowShouldClose();
		}
	};
}

#endif