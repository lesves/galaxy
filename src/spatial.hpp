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

		bool has_nan() const {
			for (std::size_t d = 0; d < D; ++d) {
				if (std::isnan(components_[d])) {
					return true;
				}
			}
			return false;
		}
	};

	template<typename T, Dimension D>
	inline Vector<T, D> operator*(const Vector<T, D>& one, const T& two) {
		Vector<T, D> res;
		for (std::size_t d = 0; d < D; ++d) {
			res[d] = one[d] * two;
		}
		return res;
	}

	template<typename T, Dimension D>
	inline Vector<T, D> operator*(const T& two, const Vector<T, D>& one) {
		Vector<T, D> res;
		for (std::size_t d = 0; d < D; ++d) {
			res[d] = one[d] * two;
		}
		return res;
	}

	template<typename T, Dimension D>
	inline void operator*=(Vector<T, D>& one, T& two) {
		for (std::size_t d = 0; d < D; ++d) {
			one[d] *= two;
		}
	}

	template<typename T, Dimension D>
	inline Vector<T, D> operator/(const Vector<T, D>& one, const T& two) {
		Vector<T, D> res;
		for (std::size_t d = 0; d < D; ++d) {
			res[d] = one[d] / two;
		}
		return res;
	}

	template<typename T, Dimension D>
	inline Vector<T, D> operator/(const T& two, const Vector<T, D>& one) {
		Vector<T, D> res;
		for (std::size_t d = 0; d < D; ++d) {
			res[d] = one[d] / two;
		}
		return res;
	}

	template<typename T, Dimension D>
	inline void operator/=(Vector<T, D>& one, T& two) {
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

	template<typename T, Dimension N, Dimension M>
	class Matrix {
	private:
		std::array<std::array<T, M>, N> data_;

	public:
		Matrix(): data_({}) {}

		const T& operator()(std::size_t i, std::size_t j) const {
			return data_[i][j];
		}

		T& operator()(std::size_t i, std::size_t j) {
			return data_[i][j];
		}

		template<Dimension N2>
		Matrix<T, N, N2> operator*(const Matrix<T, M, N2>& other) const {
			Matrix<T, N, N2> res;
			for (int i = 0; i < N; ++i) {
				for (int j = 0; j < N2; ++j) {
					for (int k = 0; k < M; ++k) {
						res(i, j) += data_[i][k] * other(k, j);
					}
				}
			}
			return res;
		}
	};

	template<typename T, Dimension N, Dimension M>
	inline Vector<T, N> operator*(const Matrix<T, N, M>& mat, const Vector<T, M>& vec) {
		Vector<T, N> res;
		for (std::size_t i = 0; i < N; ++i) {
			for (std::size_t j = 0; j < M; ++j) {
				res[i] += vec[j]*mat(i, j);
			}
		}
		return res;
	}

	template<typename T, Dimension N>
	inline Matrix<T, N, N> identity() {
		Matrix<T, N, N> res;
		for (std::size_t i = 0; i < N; ++i) {
			res(i, i) = 1;
		}
	}

	template<typename T>
	inline Matrix<T, 3, 3> rotation_x(T x) {
		Matrix<T, 3, 3> res;
		res(0, 0) = 1;
		res(1, 1) = std::cos(x);
		res(1, 2) = -std::sin(x);
		res(2, 1) = std::sin(x);
		res(2, 2) = std::cos(x);
		return res;
	}

	template<typename T>
	inline Matrix<T, 3, 3> rotation_y(T y) {
		Matrix<T, 3, 3> res;
		res(0, 0) = std::cos(y);
		res(1, 1) = 1;
		res(0, 2) = std::sin(y);
		res(2, 0) = -std::sin(y);
		res(2, 2) = std::cos(y);
		return res;
	}

	template<typename T>
	inline Matrix<T, 3, 3> rotation_z(T z) {
		Matrix<T, 3, 3> res;
		res(0, 0) = std::cos(z);
		res(0, 1) = -std::sin(z);
		res(1, 0) = std::sin(z);
		res(1, 1) = std::cos(z);
		res(2, 2) = 1;
		return res;
	}

	template<typename T>
	inline Matrix<T, 3, 3> rotation(T x, T y, T z) {
		return rotation_x(x)*rotation_y(y)*rotation_z(z);
	}
}

#endif
