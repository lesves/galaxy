#include <iostream>
#include <vector>
#include "config.hpp"
#include "simulation.hpp"
#include "mass_distribution.hpp"
#include "integration.hpp"
#include "tree_graphics.hpp"


int main(int argc, char** argv) {
	try {
		std::vector<std::string> args(argv + 1, argv + argc);

		auto mgr = config::ConfigurationManager(args.empty() ? "simulation.toml" : args[0]);
		auto cfg = mgr.get_config();
		config::Units units(cfg);

		using Body = simulation::Body2D<double, false>;
		using Graphics = tree_graphics::Graphics2D;
		using Engine = simulation::TreeSimulationEngine<Body, Graphics>;

		auto intm = integration::get<Body>(cfg);
		auto mdist = mass_distribution::get<Body>(cfg);

		Engine sim(cfg, units, intm, mdist);
		for (;;) {
			if (!sim.step()) {
				break;
			}
		}

	} catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
		return 1;
    } catch (...) {
        std::cout << "Error: Unknown exception." << std::endl;
		return 1;
    }
}