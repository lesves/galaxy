#ifndef GALAXY_SIMULATION_H
#define GALAXY_SIMULATION_H

#include "orthtree.hpp"

namespace simulation {
	template<typename Policy, typename Graphics>
	class TreeSimulation {
	private:
		Policy policy_;
		Graphics graphics_;

	public:
		std::vector<typename Policy::Body> bodies;
		typename Policy::NumType time = 0;

		TreeSimulation(const Policy& policy, const Graphics& graphics): graphics_(graphics), TreeSimulation(policy) {}
		TreeSimulation(const Policy& policy): policy_(policy), TreeSimulation() {}
		TreeSimulation(): bodies(policy_.mass_distribution(policy_)) {}

		typename Policy::Vector interact(const typename Policy::Body& body, const typename Policy::Point& other_pos, typename Policy::NumType other_mass) const {
			auto diff = body.pos - other_pos;

			auto dist = diff.norm();
			auto smoothed = sqrt(dist*dist + policy_.eps*policy_.eps);

			auto acc = -policy_.G * other_mass * diff / std::pow(smoothed, 3);
			return acc;
		}

		typename Policy::Vector traverse(const typename Policy::Body& body, typename Policy::OrthTree::Node* node) const {
			typename Policy::Vector res;

			auto mc = node->accum_value.center_of_mass();
			auto d = (body.pos-mc).norm();

			if (node->bbox.s() < policy_.theta*d) {
				res = interact(body, mc, node->accum_value.total_mass);
			} else {
				if (node->is_leaf()) {
					for (auto&& other : node->data) {
						res += interact(body, other.pos, other.mass);
					}
				} else {
					for (auto&& child : *(node->children)) {
						res += traverse(body, child.get());
					}
				}
			}

			return res;
		}

		bool step() {
			typename Policy::OrthTree tree(policy_.tree_policy, policy_.bbox);

			for (auto&& body : bodies) {
				tree.insert(body);
			}

			graphics_.show(time, policy_, tree);
			if (graphics_.poll_close()) {
				return false;
			}

			for (auto& body : bodies) {
				auto acc = traverse(body, &tree.root());
				body.vel += acc*policy_.dt;
			}

			for (auto& body : bodies) {
				body.pos += body.vel*policy_.dt;
			}

			time += policy_.dt;

			typename Policy::Vector vel_mean;
			for (auto&& body : bodies) {
				vel_mean += body.vel;
			}
			auto pos_mean = tree.root().accum_value.center_of_mass();
			vel_mean /= tree.root().accum_value.total_mass;

			for (auto& body : bodies) {
				body.pos -= pos_mean;
				body.vel -= vel_mean;
			}

			return true;
		}
	};
}

#endif