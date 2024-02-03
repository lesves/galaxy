#ifndef GALAXY_SIMULATION_H
#define GALAXY_SIMULATION_H

#include "mass_distribution.hpp"
#include "integration.hpp"
#include "orthtree.hpp"
#include "spatial.hpp"
#include "config.hpp"
#include "plots.hpp"
#include <utility>

#include <numbers>


namespace simulation {
	template<typename NT, spatial::Dimension D, bool include>
	struct ConditionalAcc;

	template<typename NT, spatial::Dimension D>
	struct ConditionalAcc<NT, D, false> {};

	template<typename NT, spatial::Dimension D>
	struct ConditionalAcc<NT, D, true> {
		spatial::Vector<NT, D> acc;
	};

	template<typename NumType, spatial::Dimension D, bool store_acc = true>
	struct Body : public ConditionalAcc<NumType, D, store_acc> {
		static constexpr spatial::Dimension Dim = D;
		using Scalar = NumType;
		using Vector = spatial::Vector<NumType, D>;
		using Point = spatial::Point<NumType, D>;

		struct GetPoint {
			Point operator()(const Body& body) const {
				return body.pos;
			}
		};

		Point pos;
		Vector vel;
		Scalar mass;

		Body(const Point& pos, const Vector& vel, Scalar mass): pos(pos), vel(vel), mass(mass) {};
	};

	template<typename NumType, bool store_acc = false>
	using Body2D = Body<NumType, 2, store_acc>;

	template<typename Body, typename Graphics>
	class TreeSimulationEngine {
	private:
		using Scalar = typename Body::Scalar;
		using Vector = typename Body::Vector;
		using Point = typename Body::Point;

		struct TreePolicy {
			using Item = Body;
			using NumType = typename Body::Scalar;
			using GetPoint = typename Body::GetPoint;

			static constexpr bool use_accum = true;
			struct AccumType {
				std::size_t count = 0;
				Vector pos_sum;

				Scalar total_mass = 0;

				Point center_of_mass() const {
					return pos_sum/(Scalar)count;
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
		using TreeType = orthtree::OrthTree<Body, Body::Dim, TreePolicy>;
		
		integration::IntegrationMethod<Body> integration_;
		Graphics graphics_;

		bool plot_energy_;

		// TODO: Move as constructor to OrthTree
		TreeType build_tree() {
			TreeType tree(tree_policy, bbox);

			for (auto&& body : bodies) {
				tree.insert(body);
			}

			return tree;
		}

		std::pair<Vector, Scalar> interact(const Body& body, const Point& other_pos, Scalar other_mass) const {
			auto diff = body.pos - other_pos;

			auto dist = diff.norm();
			auto smoothed = sqrt(dist*dist + eps*eps);

			auto acc = -G * other_mass * diff / std::pow(smoothed, 3);
			auto pot = -G * body.mass * other_mass / smoothed / 2;

			return std::make_pair(acc, pot);
		}

		std::pair<Vector, Scalar> traverse(const Body& body, typename TreeType::Node* node) const {
			Vector res_acc;
			Scalar res_pot = 0.;

			auto mc = node->accum_value.center_of_mass();
			auto d = (body.pos-mc).norm();

			if (node->bbox.s() < theta*d) {
				//assert(!node->bbox.contains(body.pos));
				auto [acc, pot] = interact(body, mc, node->accum_value.total_mass);
				res_acc += acc;
				res_pot += pot;
			} else {
				if (node->is_leaf()) {
					for (auto&& other : node->data) {
						auto [acc, pot] = interact(body, other.pos, other.mass);
						res_acc += acc;
						res_pot += pot;
					}
				} else {
					for (auto&& child : *(node->children)) {
						auto [acc, pot] = traverse(body, child.get());
						res_acc += acc;
						res_pot += pot;
					}
				}
			}

			return std::make_pair(res_acc, res_pot);
		}

		// TODO: Generalize
		void velocity_initialization(Body& body, const typename Body::Vector& acc) {
			using Scalar = typename Body::Scalar;

			Scalar a = acc.norm();

			Scalar r = body.pos.norm();
			Scalar theta = std::atan2(body.pos[1], body.pos[0]);

			auto tmp = body.pos * acc / r / a;
			Scalar cosphi = -tmp[0]-tmp[1];
			Scalar a_r = cosphi * a;
			if (a_r < 0.) a_r = 0.;

			Scalar v_t = sqrt(a_r * r);
			Scalar v_r = 0;

			body.vel[0] = v_t*std::cos(theta - std::numbers::pi/2) + v_r*std::cos(theta);
			body.vel[1] = v_t*std::sin(theta - std::numbers::pi/2) + v_r*std::sin(theta);
		}

	public:
		spatial::Box<Scalar, Body::Dim> bbox;
		std::vector<Body> bodies;
		Scalar time = 0;

		Scalar dt;
		Scalar theta;
		Scalar eps;
		Scalar G;

		plots::EnergyStatsPlot energy;

		spatial::Box<Scalar, Body::Dim> init_bbox(config::Config cfg) {
			config::Config extent_ = cfg.get_or_fail("simulation.size.extent");
			std::array<Scalar, 2> extent = { extent_.get_or_fail<Scalar>("x"), extent_.get_or_fail<Scalar>("y") };

			spatial::Point<Scalar, Body::Dim> center;
			return spatial::Box<Scalar, Body::Dim>(center, extent);
		}

		TreeSimulationEngine(config::Config cfg, config::Units& units, integration::IntegrationMethod<Body> intm, mass_distribution::MassDistribution<Body> mdist): 
				integration_(intm), 
				graphics_(cfg, units), 
				bodies(mdist(cfg)),
				bbox(init_bbox(cfg)),
				energy(cfg)
		{
			plot_energy_ = cfg.get("simulation.plots.energy").has_value();

			G = units.G();
			theta = cfg.get_or_fail<double>("simulation.engine.theta");
			eps = cfg.get_or_fail<double>("simulation.engine.eps");

			dt = cfg.get_or_fail<double>("simulation.integration.dt");

			setup();
		}

		void setup() {
			TreeType tree = build_tree();

			for (auto& body : bodies) {
				auto [acc, _] = traverse(body, &tree.root());
				velocity_initialization(body, acc);
			}

			// Centroidal coordinates
			Vector vel_mean;
			for (auto&& body : bodies) {
				vel_mean += body.vel;
			}
			vel_mean /= tree.root().accum_value.total_mass;
			auto pos_mean = tree.root().accum_value.center_of_mass();

			for (auto& body : bodies) {
				body.pos -= pos_mean;
				body.vel -= vel_mean;
			}
		}

		bool step() {
			TreeType tree = build_tree();

			// Calculate accelerations
			std::vector<Vector> accelerations;
			Scalar pot_energy = 0.;
			for (auto&& body : bodies) {
				auto [acc, pot] = traverse(body, &tree.root());
				pot_energy += pot;
				accelerations.push_back(acc);
			}

			// Do graphics
			if (plot_energy_) {
				Scalar kin_energy = 0.;

				for (std::size_t i = 0; i < bodies.size(); ++i) {
					kin_energy += 0.5 * bodies[i].mass * bodies[i].vel.norm_squared();
				}

				energy.log(kin_energy, pot_energy);
				energy.show();
			}
			
			graphics_.show(time, tree);

			if (graphics_.poll_close()) {
				return false;
			}

			// Integrate
			for (std::size_t i = 0; i < bodies.size(); ++i) {
				integration_(bodies[i], dt, accelerations[i]);
			}
			time += dt;

			return true;
		}
	};
}

#endif