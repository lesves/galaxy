#ifndef GALAXY_MASS_DISTRIBUTION_H
#define GALAXY_MASS_DISTRIBUTION_H

#include <numbers>
#include <random>
#include <functional>
#include <vector>
#include "config.hpp"


namespace mass_distribution {
	template<typename Body, typename Engine>
	using MassDistribution = std::function<void(config::Config, Engine*)>;

	double deg2rad(double deg) {
		return deg * std::numbers::pi/180;
	}

	template<typename Body>
	void transform(config::Config mcfg, std::vector<Body>& bodies) {
		double offset_x = mcfg.get<double>("offset.x").value_or(0.);
		double offset_y = mcfg.get<double>("offset.y").value_or(0.);

		typename Body::Vector offset;
		if constexpr (Body::Dim >= 3) {
			double offset_z = mcfg.get<double>("offset.z").value_or(0.);
			offset = typename Body::Vector({offset_x, offset_y, offset_z});

			double rot_x = deg2rad(mcfg.get<double>("rotation.x").value_or(0.));
			double rot_y = deg2rad(mcfg.get<double>("rotation.y").value_or(0.));
			double rot_z = deg2rad(mcfg.get<double>("rotation.y").value_or(0.));

			auto rmat = spatial::rotation_x(rot_x);//, rot_y, rot_z);
			for (auto& body : bodies) {
				body.pos = rmat*body.pos;
				body.vel = rmat*body.vel;
			}

		} else {
			offset = typename Body::Vector({offset_x, offset_y});
		}

		for (auto& body : bodies) {
			body.pos += offset;
		}
	}

	template<typename Body, typename Engine>
	void test_case_1(config::Config mcfg, Engine* eng) {
		double total_mass = mcfg.get_or_fail<double>("total_mass");

		typename Body::Vector vel({0, 0});

		auto prev_size = eng->bodies.size();
		eng->bodies.emplace_back(typename Body::Point({-20., 0.}), vel, total_mass/2);
		eng->bodies.emplace_back(typename Body::Point({ 20., 0.}), vel, total_mass/2);

		eng->init_vels(eng->bodies.begin()+prev_size, eng->bodies.end());
		transform(mcfg, eng->bodies);
	}

	template<typename Body, typename Engine>
	void simple_exponential(config::Config mcfg, Engine* eng) {
		std::size_t N = mcfg.get_or_fail<std::size_t>("N");
		double total_mass = mcfg.get_or_fail<double>("total_mass");
		double lambda = mcfg.get_or_fail<double>("lambda");

		std::uniform_real_distribution<typename Body::Scalar> ang_dist(-std::numbers::pi, std::numbers::pi);
		std::exponential_distribution<typename Body::Scalar> r_dist(lambda);
		std::default_random_engine re;

		auto prev_size = eng->bodies.size();
		for (std::size_t i = 0; i < N; ++i) {
			auto ang = ang_dist(re);
			auto r = r_dist(re);

			typename Body::Point pos({std::cos(ang)*r, std::sin(ang)*r});
			typename Body::Vector vel({0, 0});
			eng->bodies.emplace_back(pos, vel, total_mass/N);
		}

		eng->init_vels(eng->bodies.begin()+prev_size, eng->bodies.end());
		transform(mcfg, eng->bodies);
	}

	template<typename Body, typename Engine>
	void simple_exponential_sphere(config::Config mcfg, Engine* eng) {
		std::size_t N = mcfg.get_or_fail<std::size_t>("N");
		double total_mass = mcfg.get_or_fail<double>("total_mass");
		double lambda = mcfg.get_or_fail<double>("lambda");

		std::uniform_real_distribution<typename Body::Scalar> ang1_dist(-std::numbers::pi, std::numbers::pi);
		std::uniform_real_distribution<typename Body::Scalar> ang2_dist(-std::numbers::pi, std::numbers::pi);
		std::exponential_distribution<typename Body::Scalar> r_dist(lambda);
		std::default_random_engine re;

		auto prev_size = eng->bodies.size();
		for (std::size_t i = 0; i < N; ++i) {
			auto ang1 = ang1_dist(re);
			auto ang2 = ang2_dist(re);
			auto r = r_dist(re);

			typename Body::Point pos({std::sin(ang1)*std::cos(ang2)*r, std::sin(ang1)*std::sin(ang2)*r, std::cos(ang1)*r});
			typename Body::Vector vel({0, 0, 0});
			eng->bodies.emplace_back(pos, vel, total_mass/N);
		}
		eng->init_vels(eng->bodies.begin()+prev_size, eng->bodies.end());
		transform(mcfg, eng->bodies);
	}

	template<typename Body, typename Engine>
	MassDistribution<Body, Engine> get(config::Config mcfg);

	template<typename Body, typename Engine>
	void composite(config::Config mcfg, Engine* eng) {
		for (auto&& cfg : mcfg.get_configs("composite")) {
			get<Body, Engine>(cfg)(cfg, eng);
		}
	}

	template<typename Body, typename Engine>
	MassDistribution<Body, Engine> get(config::Config mcfg) {
		auto name = mcfg.get_or_fail<std::string>("type");

		if constexpr (Body::Dim >= 3) {
			if (name == "simple_exponential_sphere") {
				return simple_exponential_sphere<Body, Engine>;
			}
		}

		if (name == "test_case_1") {
			return test_case_1<Body, Engine>;
		} else if (name == "simple_exponential") {
			return simple_exponential<Body, Engine>;
		} else if (name == "composite") {
			return composite<Body, Engine>;
		} else {
			config::backend_fail("mass_distribution");
			return {};
		}
	}
}

#endif