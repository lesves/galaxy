#ifndef GALAXY_SIMULATION_H
#define GALAXY_SIMULATION_H

#include "orthtree.hpp"
#include <utility>

namespace simulation {
	template<typename NumType>
	struct Stats {
		std::vector<NumType> kin_energy;
		std::vector<NumType> pot_energy;
	};

	template<typename Policy, typename Graphics>
	class TreeSimulation {
	private:
		Policy policy_;
		Graphics graphics_;

		struct TreePolicy {
			using NumType = typename Policy::NumType;
			using GetPoint = typename Policy::GetPoint;

			static constexpr bool use_accum = true;
			struct AccumType {
				std::size_t count = 0;
				typename Policy::Vector pos_sum;

				NumType total_mass = 0;

				typename Policy::Point center_of_mass() const {
					return pos_sum/(NumType)count;
				}
			};
			struct Accum {
				void operator()(AccumType& cur, const typename Policy::Body& body) const {
					cur.count += 1;
					cur.pos_sum += body.pos;
					cur.total_mass += body.mass;
				}
			};

			std::size_t node_capacity = 1;
		} tree_policy;

		using TreeType = orthtree::OrthTree<typename Policy::Body, Policy::Dim, TreePolicy>;

		TreeType build_tree() {
			TreeType tree(tree_policy, policy_.bbox);

			for (auto&& body : bodies) {
				tree.insert(body);
			}

			return tree;
		}

	public:
		std::vector<typename Policy::Body> bodies;
		typename Policy::NumType time = 0;

		Stats<typename Policy::NumType> stats;

		TreeSimulation(const Policy& policy, const Graphics& graphics): graphics_(graphics), TreeSimulation(policy) {}
		TreeSimulation(const Policy& policy): policy_(policy), TreeSimulation() {}
		TreeSimulation(): bodies(policy_.mass_distribution(policy_)) {}

		std::pair<typename Policy::Vector, typename Policy::NumType> interact(const typename Policy::Body& body, const typename Policy::Point& other_pos, typename Policy::NumType other_mass) const {
			auto diff = body.pos - other_pos;

			auto dist = diff.norm();
			auto smoothed = sqrt(dist*dist + policy_.eps*policy_.eps);

			auto acc = -policy_.G * other_mass * diff / std::pow(smoothed, 3);
			auto pot = -policy_.G * body.mass * other_mass / smoothed / 2;

			return std::make_pair(acc, pot);
		}

		std::pair<typename Policy::Vector, typename Policy::NumType> traverse(const typename Policy::Body& body, typename TreeType::Node* node) const {
			typename Policy::Vector res_acc;
			typename Policy::NumType res_pot = 0.;

			auto mc = node->accum_value.center_of_mass();
			auto d = (body.pos-mc).norm();

			if (node->bbox.s() < policy_.theta*d) {
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

		void init() {
			TreeType tree = build_tree();

			for (auto& body : bodies) {
				auto [acc, _] = traverse(body, &tree.root());
				policy_.velocity_initialization(body, acc);
			}

			// Centroidal coordinates
			typename Policy::Vector vel_mean;
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

			graphics_.show(time, policy_, tree);
			if (graphics_.poll_close()) {
				return false;
			}

			std::vector<typename Policy::Vector> accelerations;
			typename Policy::NumType pot_energy = 0.;
			for (auto&& body : bodies) {
				auto [acc, pot] = traverse(body, &tree.root());
				pot_energy += pot;
				accelerations.push_back(acc);
			}

			for (std::size_t i = 0; i < bodies.size(); ++i) {
				policy_.integration(policy_, bodies[i], accelerations[i]);
			}

			time += policy_.dt;

			typename Policy::NumType kin_energy = 0.;
			if (policy_.plot_energy) {
				for (std::size_t i = 0; i < bodies.size(); ++i) {
					kin_energy += 0.5 * bodies[i].mass * bodies[i].vel.norm_squared();
				}

				stats.kin_energy.push_back(kin_energy);
				stats.pot_energy.push_back(pot_energy);

				graphics_.plot(policy_, stats);
			}

			return true;
		}
	};
}

#endif