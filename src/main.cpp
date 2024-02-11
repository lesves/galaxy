#include <iostream>
#include <vector>
#include <csignal>
#include "config.hpp"
#include "spatial.hpp"
#include "simulation.hpp"
#include "mass_distribution.hpp"
#include "integration.hpp"

#ifdef USE_OPENCV_GRAPHICS
	#include "graphics/opencv/graphics_2d.hpp"
	#include "graphics/opencv/graphics_3d.hpp"

	#include "graphics/opencv/video.hpp"
#else
	#include "graphics/raylib/graphics_2d.hpp"
	#include "graphics/raylib/graphics_3d.hpp"
#endif


namespace signals {
	static volatile std::sig_atomic_t signal_status;
	static volatile bool ok_status = true;

	void signal_handler(int signal) {
		signal_status = signal;
		ok_status = false;
	}
}


template<typename Body, typename Graphics>
void run(config::Config cfg, const config::Units& units) {
	using Engine = simulation::TreeSimulationEngine<Body, Graphics>;

	auto intm = integration::get<Body>(cfg.get_or_fail("simulation.integration"));
	auto mdist = mass_distribution::get<Body, Engine>(cfg.get_or_fail("simulation.mass_distribution"));

	Engine sim(cfg, units, intm, mdist);

	while(signals::ok_status) {
		if (!sim.step()) {
			break;
		}
	}
	#ifdef USE_OPENCV_GRAPHICS
		video::Writer::handle_exit();
	#endif
}


int main(int argc, char** argv) {
	try {
		std::signal(SIGINT, signals::signal_handler);

		std::vector<std::string> args(argv + 1, argv + argc);

		auto mgr = config::ConfigurationManager(args.empty() ? "simulation.toml" : args[0]);
		auto cfg = mgr.get_config();
		config::Units units(cfg);

		auto dim = cfg.get_or_fail<spatial::Dimension>("simulation.dim");

		if (dim == 2) {
			run<simulation::Body2D<double>, graphics::Graphics2D>(cfg, units);
		} else if (dim == 3) {
			run<simulation::Body3D<double>, graphics::Graphics3D>(cfg, units);
		} else {
			throw config::configuration_error("Unsupported simulation dimension.");
		}

	} catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
		return 1;
    } catch (...) {
        std::cout << "Error: Unknown exception." << std::endl;
		return 1;
    }
}