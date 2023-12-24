#include <iostream>
#include <vector>

#include <random>

#include "orthtree.hpp"
#include "graphics.hpp"


template<typename Policy>
std::vector<orthtree::Point<double, 2>> dummy_distribution(Policy policy) {
	std::vector<orthtree::Point<double, 2>> res;

	std::uniform_real_distribution<double> gen_x(-policy.extent_x, policy.extent_x);
	std::uniform_real_distribution<double> gen_y(-policy.extent_y, policy.extent_y);
	std::default_random_engine re;

	for (std::size_t i = 0; i < policy.N; ++i) {
		for (;;) {
			auto x = gen_x(re);
			auto y = gen_y(re);

			if (x*x + y*y <= policy.disk_size) {
				res.push_back(orthtree::Point<double, 2>({x, y}));
				break;
			}
		}
	}

	return res;
}


class SimulationPolicy {
public:
	// OrthTree settings
	using NumType = double;
	static constexpr std::size_t Dim = 2;
	std::size_t node_capacity = 10;

	// Physical constants & units
	double G0 = 6.67430E-11;
	double Myear = 60*60*24*365*1000000.;
	double mass_sun = 1.989E30;
	double parsec = 30856775810000000;

	double shown_unit = parsec*1000;
	std::string shown_unit_name = "kpc";

	double scale = 0.1;
	double unit = shown_unit*scale;

	double G = G0 * (Myear*Myear) / (unit*unit*unit) * mass_sun;

	// Simulation size
	double extent_x = 100;
	double extent_y = 100;
	std::array<double, 2> extent = {extent_x, extent_y};

	orthtree::Point<NumType, Dim> center;
	orthtree::Box<NumType, Dim> bbox = orthtree::Box<NumType, Dim>(center, extent);

	// Video output settings
	std::size_t width = (std::size_t)extent_x*2;
	std::size_t height = (std::size_t)extent_y*2;
	std::size_t display_scale = 3;

	// Mass distribution parameters
	std::size_t N = 1000;
	double total_mass = 1E11;

	static constexpr auto mass_distribution = dummy_distribution<SimulationPolicy>;
	double disk_size = 3600;

	// Simulation parameters
	double dt = 1;
};


int main(int argc, char** argv) {
	try {
		std::vector<std::string> args(argv + 1, argv + argc);

		SimulationPolicy pol;

		orthtree::QuadTree<double, SimulationPolicy> qt(pol, pol.bbox);

		std::vector<orthtree::Point<double, 2>> points = dummy_distribution(pol);
		for (auto&& pt : points) {
			qt.insert(pt);
		}

		graphics2d::show(pol, qt);

	} catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
		return 1;
    } catch (...) {
        std::cout << "Unknown exception." << std::endl;
		return 1;
    }
}
