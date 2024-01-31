#ifndef GALAXY_SPATIAL_H
#define GALAXY_SPATIAL_H
#include <algorithm>

namespace spatial {
	using Dimension = std::size_t;

	template<typename T, Dimension D>
	class Vector {
	private:
		std::array<T, D> components_;

	public:
		static constexpr spatial::Dimension Dim = D;

		Vector() : components_({}) {};
		Vector(const std::array<T, D>& components) : components_(components) {};
		Vector(std::array<T, D>&& components) : components_(std::move(components)) {};

		const T& operator[](std::size_t idx) const {
			return components_[idx];
		}

		T& operator[](std::size_t idx) {
			return components_[idx];
		}

		typename std::array<T, D>::iterator begin() {
			return components_.begin();
		}

		typename std::array<T, D>::iterator end() {
			return components_.end();
		}

		typename std::array<T, D>::const_iterator cbegin() const {
			return components_.cbegin();
		}

		typename std::array<T, D>::const_iterator cend() const {
			return components_.cend();
		}

		Vector<T, D> operator+(const Vector<T, D>& other) const {
			Vector<T, D> res;
			for (std::size_t d = 0; d < D; ++d) {
				res[d] = components_[d] + other[d];
			}
			return res;
		}

		void operator+=(const Vector<T, D>& other) {
			for (std::size_t d = 0; d < D; ++d) {
				components_[d] += other[d];
			}
		}

		Vector<T, D> operator-(const Vector<T, D>& other) const {
			Vector<T, D> res;
			for (std::size_t d = 0; d < D; ++d) {
				res[d] = components_[d] - other[d];
			}
			return res;
		}

		void operator-=(const Vector<T, D>& other) {
			for (std::size_t d = 0; d < D; ++d) {
				components_[d] -= other[d];
			}
		}

		Vector<T, D> operator*(const Vector<T, D>& other) const {
			Vector<T, D> res;
			for (std::size_t d = 0; d < D; ++d) {
				res[d] = components_[d] * other[d];
			}
			return res;
		}

		void operator*=(const Vector<T, D>& other) {
			for (std::size_t d = 0; d < D; ++d) {
				components_[d] *= other[d];
			}
		}

		Vector<T, D> sqrt() const {
			Vector<T, D> res;
			for (std::size_t d = 0; d < D; ++d) {
				res = std::sqrt(components_[d]);
			}
		}

		T norm_squared() const {
			T res = 0;
			for (std::size_t d = 0; d < D; ++d) {
				res += components_[d]*components_[d];
			}
			return res;
		}

		T norm() const {
			return std::sqrt(norm_squared());
		}
	};

	template<typename T, Dimension D>
	Vector<T, D> operator*(const Vector<T, D>& one, const T& two) {
		Vector<T, D> res;
		for (std::size_t d = 0; d < D; ++d) {
			res[d] = one[d] * two;
		}
		return res;
	}

	template<typename T, Dimension D>
	Vector<T, D> operator*(const T& two, const Vector<T, D>& one) {
		Vector<T, D> res;
		for (std::size_t d = 0; d < D; ++d) {
			res[d] = one[d] * two;
		}
		return res;
	}

	template<typename T, Dimension D>
	void operator*=(Vector<T, D>& one, T& two) {
		for (std::size_t d = 0; d < D; ++d) {
			one[d] *= two;
		}
	}

	template<typename T, Dimension D>
	Vector<T, D> operator/(const Vector<T, D>& one, const T& two) {
		Vector<T, D> res;
		for (std::size_t d = 0; d < D; ++d) {
			res[d] = one[d] / two;
		}
		return res;
	}

	template<typename T, Dimension D>
	Vector<T, D> operator/(const T& two, const Vector<T, D>& one) {
		Vector<T, D> res;
		for (std::size_t d = 0; d < D; ++d) {
			res[d] = one[d] / two;
		}
		return res;
	}

	template<typename T, Dimension D>
	void operator/=(Vector<T, D>& one, T& two) {
		for (std::size_t d = 0; d < D; ++d) {
			one[d] /= two;
		}
	}

	template<typename T, Dimension D>
	using Point = Vector<T, D>;

	template<typename T, Dimension D>
	struct Box {
		Point<T, D> center;
		Vector<T, D> extent;

		Box(const Point<T, D>& center, T ext_same): center(center) {
			extent.fill(ext_same);
		};
		Box(const Point<T, D>& center, const Vector<T, D>& extent): center(center), extent(extent) {};

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

		T s() const {
			return *std::max_element(extent.cbegin(), extent.cend());
		}
	};
}

#endif
