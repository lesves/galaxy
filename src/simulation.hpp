#ifndef GALAXY_SIMULATION_H
#define GALAXY_SIMULATION_H

#include "mass_distribution.hpp"
#include "integration.hpp"
#include "orthtree.hpp"
#include "spatial.hpp"
#include "config.hpp"
#include <utility>

#include "graphics/plots.hpp"


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

	template<typename NumType, bool store_acc = false>
	using Body3D = Body<NumType, 3, store_acc>;

	template<typename Body, typename Graphics>
	class TreeSimulationEngine {
	public:
		using Scalar = typename Body::Scalar;
		using Vector = typename Body::Vector;
		using Point = typename Body::Point;
		
	private:
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

		std::pair<Vector, Scalar> interact(const Body& body, const Point& other_pos, Scalar other_mass) const {
			auto diff = body.pos - other_pos;

			auto dist = diff.norm();
			auto smoothed = std::sqrt(dist*dist + eps*eps);

			auto acc = -G * other_mass * diff / std::pow(smoothed, (Scalar)3);
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
					/*std::vector<std::pair<Vector, Scalar>> res(node->children.value().size());
					std::transform(
						std::execution::par, 
						node->children.value().begin(), node->children.value().end(),
						res.begin(),
						[this, &body](auto&& child) {
							return traverse(body, child.get());
						}
					);
					for (auto&& [acc, pot] : res) {
						res_acc += acc;
						res_pot += pot;
					}*/
				}
			}

			return std::make_pair(res_acc, res_pot);
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
			std::array<Scalar, Body::Dim> extent = config::get_coords_or_fail<Scalar, Body::Dim>()(cfg, "simulation.size.extent");

			spatial::Point<Scalar, Body::Dim> center;
			return spatial::Box<Scalar, Body::Dim>(center, extent);
		}

		TreeSimulationEngine(config::Config cfg, const config::Units& units, integration::IntegrationMethod<Body> intm, mass_distribution::MassDistribution<Body, TreeSimulationEngine<Body, Graphics>> mdist): 
				integration_(intm), 
				graphics_(cfg, units),
				bbox(init_bbox(cfg)),
				energy(cfg)
		{
			plot_energy_ = cfg.get<bool>("simulation.plots.energy.enable").value_or(true);

			G = units.G();
			theta = cfg.get_or_fail<Scalar>("simulation.engine.theta");
			eps = cfg.get_or_fail<Scalar>("simulation.engine.eps");

			dt = cfg.get_or_fail<Scalar>("simulation.integration.dt");

			mdist(cfg.get_or_fail("simulation.mass_distribution"), this);
		}

		static void velocity_initialization(Body& body, const Vector& acc) {
			Scalar a = acc.norm();

			Scalar r = body.pos.norm();
			Scalar theta = std::atan2(body.pos[1], body.pos[0]);

			auto tmp = body.pos * acc / r / a;
			Scalar cosphi = -tmp[0]-tmp[1];
			Scalar a_r = cosphi * a;
			if (a_r < 0.) a_r = 0.;

			Scalar v_t = std::sqrt(a_r * r);
			Scalar v_r = 0;

			body.vel[0] = v_t*std::cos(theta - std::numbers::pi/2) + v_r*std::cos(theta);
			body.vel[1] = v_t*std::sin(theta - std::numbers::pi/2) + v_r*std::sin(theta);
		}

		void init_vels(typename std::vector<Body>::iterator begin, typename std::vector<Body>::iterator end) {
			TreeType tree(tree_policy, bbox, begin, end);

			for (auto it = begin; it != end; ++it) {
				auto [acc, _] = traverse(*it, &tree.root());
				velocity_initialization(*it, acc);
			}
		}

		bool step() {
			TreeType tree(tree_policy, bbox, bodies);

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

			graphics_.show(time, this, tree);

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