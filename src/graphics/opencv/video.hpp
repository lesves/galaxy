#ifndef GALAXY_VIDEO_H
#define GALAXY_VIDEO_H

#include <opencv2/videoio.hpp>
#include <unordered_set>
#include "../../config.hpp"


namespace video {
	class Writer {
	private:
		cv::VideoWriter writer_;

		inline static std::unordered_set<Writer*> registry;

	public:
		Writer() {};
		Writer(config::Config cfg, std::size_t width, std::size_t height) {
			auto filename = cfg.get_or_fail<std::string>("simulation.video.output.file");
			auto fourcc = cfg.get<std::string>("simulation.video.output.fourcc").value_or("mp4v");

			if (fourcc.size() != 4) {
				throw config::configuration_error("Invalid fourcc code.");
			}

			auto fps = cfg.get_or_fail<double>("simulation.video.output.fps");

			std::cout << "[video::Writer] Info: Opening file '" << filename  << "'.\n";

			writer_ = cv::VideoWriter(filename, cv::VideoWriter::fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3]), fps, cv::Size(width, height));

			registry.insert(this);
		}

		~Writer() {
			registry.erase(this);
		}
		
		void write(const cv::Mat& img) {
			writer_.write(img);
		}

		static void handle_exit() {
			std::cout << "[video::Writer] Info: Closing open video writers.\n";
			std::unordered_set<Writer*> registry_copy = registry;
			for (auto&& w : registry_copy) {
				w->~Writer();
			}
		}
	};
}

#endif