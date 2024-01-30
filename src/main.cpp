#include <iostream>
#include <vector>

#include <random>

#include "simulation.hpp"
#include "spatial.hpp"
#include "orthtree.hpp"
#include "graphics.hpp"


template<typename Policy>
std::vector<typename Policy::Body> dummy_distribution(Policy policy) {
	std::vector<typename Policy::Body> res;

	std::uniform_real_distribution<typename Policy::NumType> gen_x(-policy.extent_x, policy.extent_x);
	std::uniform_real_distribution<typename Policy::NumType> gen_y(-policy.extent_y, policy.extent_y);
	std::default_random_engine re;

	for (std::size_t i = 0; i < policy.N; ++i) {
		for (;;) {
			auto x = gen_x(re);
			auto y = gen_y(re);

			if (x*x + y*y <= policy.disk_size) {
				typename Policy::Point pos({x, y});
				typename Policy::Point vel({0, 0});

				res.emplace_back(pos, vel, policy.total_mass/policy.N);
				break;
			}
		}
	}

	return res;
}

/*template<typename Policy>
void basic_velocities(std::vector<typename Policy::Body>& bodies) {
	for (auto body& : bodies) {
		auto a = body.
	}
}*/


class SimulationPolicy {
public:
	// OrthTree settings
	using NumType = double;
	static constexpr std::size_t Dim = 2;

	using Vector = spatial::Vector<NumType, Dim>;
	using Point = spatial::Point<NumType, Dim>;
	struct Body {
		Point pos;
		Vector vel;
		NumType mass;

		Body(const Point& pos, const Vector& vel, NumType mass): pos(pos), vel(vel), mass(mass) {};
	};

	struct TreePolicy {
		using NumType = NumType;

		struct GetPoint {
			Point operator()(const Body& body) const {
				return body.pos;
			}
		};

		static constexpr bool use_accum = true;
		struct AccumType {
			std::size_t count = 0;
			Vector pos_sum;

			NumType total_mass = 0;

			Point center_of_mass() const {
				return pos_sum/(NumType)count;
			}
		};
		struct Accum {
			void operator()(AccumType& cur, const Body& body) const {
				cur.count += 1;
				cur.pos_sum += body.pos;
				cur.total_mass += body.mass;
			}
		};

		std::size_t node_capacity = 1;
	} tree_policy;

	using OrthTree = orthtree::OrthTree<Body, Dim, TreePolicy>;

	// === Physical constants & units ===
	double G0 = 6.67430E-11;
	double Myear = 60*60*24*365*1000000.;
	double mass_sun = 1.989E30;
	double parsec = 30856775810000000.;

	double shown_unit = parsec*1000.;
	std::string shown_unit_name = "kpc";

	double scale = 0.1;
	double unit = shown_unit*scale;

	double G = G0 * (Myear*Myear) / (unit*unit*unit) * mass_sun;

	// === Simulation size ===
	double extent_x = 300;
	double extent_y = 300;
	std::array<double, 2> extent = {extent_x, extent_y};

	spatial::Point<double, Dim> center;
	spatial::Box<double, Dim> bbox = spatial::Box<double, Dim>(center, extent);

	// === Video output settings ===
	std::size_t width = (std::size_t)extent_x*2;
	std::size_t height = (std::size_t)extent_y*2;
	std::size_t display_scale = 3;

	// === Mass distribution parameters ===

	// number of particles
	std::size_t N = 1000;
	// total mass of particles
	double total_mass = 1E11;

	// chosen mass distribution and its parameters
	static constexpr auto mass_distribution = dummy_distribution<SimulationPolicy>;
	double disk_size = 3600;

	//static constexpr auto velocity_initialization = basic_velocities<SimulationPolicy>;

	// === Simulation parameters ===

	// the time step
	double dt = 1;

	// bodies at angles smaller than theta are neglected
	double theta = 0.2;

	// Plummer sphere's epsilon
	double eps = 0.25/scale;
};


int main(int argc, char** argv) {
	try {
		std::vector<std::string> args(argv + 1, argv + argc);

		simulation::TreeSimulation<SimulationPolicy, Graphics2D> sim;
		for (;;) {
			if (!sim.step()) {
				break;
			}
		}

	} catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
		return 1;
    } catch (...) {
        std::cout << "Unknown exception." << std::endl;
		return 1;
    }
}
