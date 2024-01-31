#include <iostream>
#include <vector>

#include <random>
#include <numbers>

#include "simulation.hpp"
#include "spatial.hpp"
#include "orthtree.hpp"
#include "graphics.hpp"


template<typename Policy>
std::vector<typename Policy::Body> test_case_mass_distribution_1(Policy policy) {
	std::vector<typename Policy::Body> res;

	res.emplace_back(typename Policy::Point({-20., 0.}), policy.total_mass/2);
	res.emplace_back(typename Policy::Point({ 20., 0.}), policy.total_mass/2);

	return res;
}

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

			if (x*x + y*y <= policy.disk_radius*policy.disk_radius) {
				typename Policy::Point pos({x, y});

				res.emplace_back(pos, policy.total_mass/policy.N);
				break;
			}
		}
	}

	return res;
}

template<typename Policy>
std::vector<typename Policy::Body> simple_exponential_distribution(Policy policy) {
	std::vector<typename Policy::Body> res;

	std::uniform_real_distribution<typename Policy::NumType> ang_dist(-std::numbers::pi, std::numbers::pi);
	std::exponential_distribution<typename Policy::NumType> r_dist(policy.lambda);
	std::default_random_engine re;

	for (std::size_t i = 0; i < policy.N; ++i) {
		auto ang = ang_dist(re);
		auto r = r_dist(re);

		typename Policy::Point pos({std::cos(ang)*r, std::sin(ang)*r});
		res.emplace_back(pos, policy.total_mass/policy.N);
	}

	return res;
}

template<typename Policy>
void basic_velocities(typename Policy::Body& body, const typename Policy::Vector& acc) {
	using NumType = typename Policy::NumType;

	NumType a = acc.norm();

	NumType r = body.pos.norm();
	NumType theta = std::atan2(body.pos[1], body.pos[0]);

	auto tmp = body.pos * acc / r / a;
	NumType cosphi = -tmp[0]-tmp[1];
	NumType a_r = cosphi * a;
	if (a_r < 0.) a_r = 0.;

	NumType v_t = sqrt(a_r * r);
	NumType v_r = 0;

	body.vel[0] = v_t*std::cos(theta - std::numbers::pi/2) + v_r*std::cos(theta);
	body.vel[1] = v_t*std::sin(theta - std::numbers::pi/2) + v_r*std::sin(theta);
}

template<typename Policy>
void euler_integration(const Policy& policy, typename Policy::Body& body, const typename Policy::Vector& acc) {
	body.vel += acc * policy.dt;
	body.pos += body.vel * policy.dt;
}

template<typename Policy>
void leapfrog_integration(const Policy& policy, typename Policy::Body& body, const typename Policy::Vector& acc) {
	auto nextvel = body.vel + acc*policy.dt*0.5;
	body.pos += nextvel*policy.dt*0.5;
	body.vel = nextvel;
}


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
		Vector acc;
		NumType mass;

		Body(const Point& pos, NumType mass): pos(pos), mass(mass) {};
	};

	struct GetPoint {
		Point operator()(const Body& body) const {
			return body.pos;
		}
	};

	// === Physical constants & units ===
	double G0 = 6.67430E-11;

	// Time units
	double Myear = 60*60*24*365*1000000.;
	double time_unit = Myear;

	// Mass units
	double mass_sun = 1.989E30;
	double mass_unit = mass_sun;

	// Distance units
	double parsec = 30856775810000000.;
	double shown_unit = parsec*1000.;
	std::string shown_unit_name = "kpc";

	double scale = 0.1;
	double unit = shown_unit*scale;

	// G recalculation
	double G = G0 * (time_unit*time_unit) / (unit*unit*unit) * mass_unit;

	// === Simulation size ===
	double extent_x = 250;
	double extent_y = 250;
	std::array<double, 2> extent = {extent_x, extent_y};

	spatial::Point<double, Dim> center;
	spatial::Box<double, Dim> bbox = spatial::Box<double, Dim>(center, extent);

	// === Video output settings ===
	std::size_t width = (std::size_t)extent_x*2;
	std::size_t height = (std::size_t)extent_y*2;
	std::size_t display_scale = 2;

	bool plot_energy = true;
	std::size_t plot_width = 500;
	std::size_t plot_height = 200;

	// === Mass distribution parameters ===

	// number of particles
	std::size_t N = 1000;
	// total mass of particles
	double total_mass = 1E11;

	// chosen mass distribution and its parameters
	static constexpr auto mass_distribution = simple_exponential_distribution<SimulationPolicy>;
	double lambda = 1/20.;

	// chosen velocity initialization and its parameters
	static constexpr auto velocity_initialization = basic_velocities<SimulationPolicy>;

	// === Simulation parameters ===

	// integration method
	static constexpr auto integration = leapfrog_integration<SimulationPolicy>;

	// the time step
	double dt = 1;

	// at angles smaller than theta we only approximate the forces
	double theta = 0.2;

	// Plummer sphere's epsilon
	double eps = 0.25/scale;
};


int main(int argc, char** argv) {
	try {
		std::vector<std::string> args(argv + 1, argv + argc);

		simulation::TreeSimulation<SimulationPolicy, Graphics2D> sim;

		sim.init();
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
