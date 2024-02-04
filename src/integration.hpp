#ifndef GALAXY_INTEGRATION_H
#define GALAXY_INTEGRATION_H
#include <functional>
#include "config.hpp"


namespace integration {
	template<typename Body>
	using IntegrationMethod = std::function<void(Body&, double, const typename Body::Vector&)>;

	template<typename Body>
	void euler(Body& body, typename Body::Scalar dt, const typename Body::Vector& acc) {
		body.vel += acc * dt;
		body.pos += body.vel * dt;
	}

	template<typename Body>
	void leapfrog(Body& body, typename Body::Scalar dt, const typename Body::Vector& acc) {
		auto nextvel = body.vel + acc*dt*(typename Body::Scalar)0.5;
		body.pos += nextvel*dt*(typename Body::Scalar)0.5;
		body.vel = nextvel;
	}

	template<typename Body>
	IntegrationMethod<Body> get(config::Config icfg) {
		auto name = icfg.get_or_fail<std::string>("type");

		if (name == "euler") {
			return euler<Body>;
		} else if (name == "leapfrog") {
			return leapfrog<Body>;
		} else {
			config::backend_fail("integration");
			return {};
		}
	}
}

#endif