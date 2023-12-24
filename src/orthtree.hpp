#ifndef ORTHTREE_H
#define ORTHTREE_H

#include <array>
#include <memory>
//#include <iostream>


namespace orthtree {
	using Dimension = std::size_t;

	template<typename T, Dimension D>
	class Point {
	private:
		std::array<T, D> coords_;

	public:
		Point() : coords_({}) {};
		Point(const std::array<T, D>& coords) : coords_(coords) {};
		Point(std::array<T, D>&& coords) : coords_(std::move(coords)) {};

		const T& operator[](std::size_t idx) const {
			return coords_[idx];
		}

		T& operator[](std::size_t idx) {
			return coords_[idx];
		}
	};

	template<typename T, Dimension D>
	class Box {
	public:
		Point<T, D> center;
		std::array<T, D> extent;

		Box(const Point<T, D>& center, T ext_same): center(center) {
			extent.fill(ext_same);
		};
		Box(const Point<T, D>& center, const std::array<T, D>& extent): center(center), extent(extent) {};

		bool contains(const Point<T, D>& pt) {
			for (std::size_t dim = 0; dim < D; ++dim) {
				if (center[dim] - extent[dim] > pt[dim] || center[dim] + extent[dim] < pt[dim]) {
					return false;
				}
			}
			return true;
		}

		bool intersects(const Box<T, D>& box) {
			for (std::size_t dim = 0; dim < D; ++dim) {
				if (center[dim]-box.center[dim] >= extent[dim]+box.extent[dim]) {
					return false;
				}
			}
			return true;
		}
	};

	class OrthTreeDefaultPolicy {
	public:
		std::size_t node_capacity;

		OrthTreeDefaultPolicy(std::size_t node_capacity) : node_capacity(node_capacity) {}
	};

	template<typename T, Dimension D, typename Policy = OrthTreeDefaultPolicy>
	class Node {
	private:
		const Policy& policy_;

	public:
		std::vector<Point<T, D>> points;
		std::optional<std::array<std::unique_ptr<Node<T, D, Policy>>, 1 << D>> children;

		Box<T, D> bbox;

		Node(const Policy& policy, Box<T, D> bbox) : policy_(policy), bbox(bbox), points() {};

		bool subdivide() {
			//std::cout << "subdivide\n";
			children.emplace();

			(*children)[0] = std::make_unique<Node>(policy_, bbox);

			for (std::size_t d = 0; d < D; ++d) {
				//std::cout << "split d=" << d << "\n";
				for (std::size_t i = 0; i < 1<<d; ++i) {
					//std::cout << "split i=" << i << "\n";
					//std::cout << "access " << i << " and " << (1<<d) + i << "\n";
					auto half = bbox.extent[d]/2;
					auto left = bbox.center[d]-half;
					auto right = bbox.center[d]+half;
					//std::cout << "half=" << half << " left=" << left << " right=" << right << "\n";

					(*children)[(1<<d) + i] = std::make_unique<Node>(policy_, (*children)[i]->bbox);

					(*children)[i]->bbox.center[d] = left;
					(*children)[i]->bbox.extent[d] = half;

					(*children)[(1<<d) + i]->bbox.center[d] = right;
					(*children)[(1<<d) + i]->bbox.extent[d] = half;
				}
			}
			//std::cout << "subdivision step ok\n";

			for (auto&& point : points) {
				auto ok = false;
				for (std::size_t i = 0; i < 2 << D; ++i) {
					if ((*children)[i]->insert(point)) {
						ok = true;
						break;
					}
				}
				if (!ok) {
					return false;
				}
			}
			points.clear();
			return true;
		}

		bool insert(const Point<T, D>& point) {
			if (!bbox.contains(point)) {
				return false;
			}

			if (children.has_value()) {
				for (auto&& child : *children) {
					if (child->insert(point)) {
						return true;
					}
				}
				return false;
			} else {
				points.push_back(point);

				if (points.size() > policy_.node_capacity) {
					return subdivide();
				} else {
					return true;
				}
			}
		}
	};

	template<typename T, Dimension D, typename Policy = OrthTreeDefaultPolicy>
	class OrthTree {
	private:
		const Policy& policy_;
		Node<T, D, Policy> root_;

	public:
		OrthTree(const Policy& policy, Box<T, D> bbox) : policy_(policy), root_(Node(policy_, bbox)) {};

		void insert(const Point<T, D>& point) {
			if (!root_.insert(point)) {
				throw std::runtime_error("unable to insert into tree");
			}
		}

		const Node<T, D, Policy>& root() const {
			return root_;
		}

		Node<T, D, Policy>& root() {
			return root_;
		}
	};

	template<typename T, typename P = OrthTreeDefaultPolicy>
	using QuadTree = OrthTree<T, 2, P>;

	template<typename T, typename P = OrthTreeDefaultPolicy>
	using OctTree = OrthTree<T, 3, P>;
}

#endif