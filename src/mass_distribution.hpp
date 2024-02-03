#ifndef GALAXY_MASS_DISTRIBUTION_H
#define GALAXY_MASS_DISTRIBUTION_H

#include <numbers>
#include <random>
#include <functional>
#include <vector>
#include "config.hpp"


namespace mass_distribution {
	template<typename Body>
	using MassDistribution = std::function<std::vector<Body>(config::Config)>;

	template<typename Body>
	std::vector<Body> test_case_1(config::Config cfg) {
		double total_mass = cfg.get_or_fail<double>("simulation.mass_distribution.total_mass");

		std::vector<Body> res;

		typename Body::Vector vel({0, 0});

		res.emplace_back(typename Body::Point({-20., 0.}), vel, total_mass/2);
		res.emplace_back(typename Body::Point({ 20., 0.}), vel, total_mass/2);

		return res;
	}

	template<typename Body>
	std::vector<Body> simple_exponential(config::Config cfg) {
		std::size_t N = cfg.get_or_fail<std::size_t>("simulation.mass_distribution.N");
		double total_mass = cfg.get_or_fail<double>("simulation.mass_distribution.total_mass");
		double lambda = cfg.get_or_fail<double>("simulation.mass_distribution.lambda");

		std::vector<Body> res;

		std::uniform_real_distribution<typename Body::Scalar> ang_dist(-std::numbers::pi, std::numbers::pi);
		std::exponential_distribution<typename Body::Scalar> r_dist(lambda);
		std::default_random_engine re;

		for (std::size_t i = 0; i < N; ++i) {
			auto ang = ang_dist(re);
			auto r = r_dist(re);

			typename Body::Point pos({std::cos(ang)*r, std::sin(ang)*r});
			typename Body::Vector vel({0, 0});
			res.emplace_back(pos, vel, total_mass/N);
		}

		return res;
	}

	template<typename Body>
	MassDistribution<Body> get(config::Config cfg) {
		auto name = cfg.get_or_fail<std::string>("simulation.mass_distribution.type");

		if (name == "test_case_1") {
			return test_case_1<Body>;
		} else if (name == "simple_exponential") {
			return simple_exponential<Body>;
		} else {
			config::backend_fail("mass_distribution");
			return {};
		}
	}
}

#endif