#ifndef GALAXY_ORTHTREE_H
#define GALAXY_ORTHTREE_H

#include <vector>

#include "spatial.hpp"

namespace orthtree {
	struct EmptyVal {};

	class OrthTreeDefaultPolicy {
	public:
		using GetPoint = std::identity;
		using AccumType = EmptyVal;

		static constexpr bool use_accum = false;
		AccumType initial;

		std::size_t node_capacity;

		OrthTreeDefaultPolicy(std::size_t node_capacity) : node_capacity(node_capacity) {}
	};

	template<typename T, spatial::Dimension Dim, typename Policy = OrthTreeDefaultPolicy>
	struct TNode {
		const Policy& policy;

		std::vector<T> data;
		typename Policy::AccumType accum_value;

		std::optional<std::array<std::unique_ptr<TNode<T, Dim, Policy>>, 1 << Dim>> children;

		spatial::Box<typename Policy::NumType, Dim> bbox;

		TNode(const Policy& policy, const spatial::Box<typename Policy::NumType, Dim>& bbox) : policy(policy), bbox(bbox) {};

		bool is_leaf() const {
			return !children.has_value();
		}

		bool subdivide() {
			//std::cout << "subdivide\n";
			children.emplace();

			(*children)[0] = std::make_unique<TNode>(policy, bbox);

			for (std::size_t d = 0; d < Dim; ++d) {
				//std::cout << "split d=" << d << "\n";
				for (std::size_t i = 0; i < 1<<d; ++i) {
					//std::cout << "split i=" << i << "\n";
					//std::cout << "access " << i << " and " << (1<<d) + i << "\n";
					auto half = bbox.extent[d]/2;
					auto left = bbox.center[d]-half;
					auto right = bbox.center[d]+half;
					//std::cout << "half=" << half << " left=" << left << " right=" << right << "\n";

					(*children)[(1<<d) + i] = std::make_unique<TNode>(policy, (*children)[i]->bbox);

					(*children)[i]->bbox.center[d] = left;
					(*children)[i]->bbox.extent[d] = half;

					(*children)[(1<<d) + i]->bbox.center[d] = right;
					(*children)[(1<<d) + i]->bbox.extent[d] = half;
				}
			}
			//std::cout << "subdivision step ok\n";

			for (auto&& value : std::move(data)) {
				auto ok = false;
				for (std::size_t i = 0; i < 2 << Dim; ++i) {
					if ((*children)[i]->insert(value)) {
						ok = true;
						break;
					}
				}
				if (!ok) {
					return false;
				}
			}
			data.clear();
			return true;
		}

		void accumulate(const T& value) {
			static constexpr typename Policy::Accum accum;

			accum(accum_value, value);
		}

		bool insert(const T& value) {
			static constexpr typename Policy::GetPoint get_point;

			if (!bbox.contains(get_point(value))) {
				return false;
			}

			//std::cout << "bbox: " << bbox.center[0] << " " << bbox.center[1] << "\n";
			//std::cout << "part: " << value.pos[0] << " " << value.pos[1] << "\n";

			if (children.has_value()) {
				for (auto&& child : *children) {
					if (child->insert(value)) {
						if constexpr (Policy::use_accum) {
							accumulate(value);
						}

						return true;
					}
				}
				return false;
			} else {
				if constexpr (Policy::use_accum) {
					accumulate(value);
				}

				data.push_back(value);

				// TODO
				if (data.size() > policy.node_capacity) {
					return subdivide();
				} else {
					return true;
				}
			}
		}

		template<typename LeafF, typename InternalF>
		void traverse(const LeafF& leaf, const InternalF& internal) {
			if (is_leaf()) {
				leaf(data, bbox);
			} else {
				internal(accum_value, bbox);
				for (auto&& child : *children) {
					child.traverse(leaf, internal);
				}
			}
		}
	};

	template<typename T, spatial::Dimension Dim, typename Policy = OrthTreeDefaultPolicy>
	class OrthTree {
	private:
		const Policy& policy_;
		TNode<T, Dim, Policy> root_;

	public:
		OrthTree(const Policy& policy, const spatial::Box<typename Policy::NumType, Dim>& bbox) : policy_(policy), root_(TNode<T, Dim, Policy>(policy_, bbox)) {};

		using Node = TNode<T, Dim, Policy>;

		bool insert(const T& value) {
			return root_.insert(value);
		}

		const TNode<T, Dim, Policy>& root() const {
			return root_;
		}

		TNode<T, Dim, Policy>& root() {
			return root_;
		}

		template<typename LeafF, typename InternalF>
		void traverse(const LeafF& leaf, const InternalF& internal) {
			root_.traverse(internal, leaf);
		}
	};

	template<typename T, typename P>
	using QuadTree = OrthTree<T, 2, P>;

	template<typename T, typename P>
	using OctTree = OrthTree<T, 3, P>;
}

#endif