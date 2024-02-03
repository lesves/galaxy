#ifndef GALAXY_INTEGRATION_H
#define GALAXY_INTEGRATION_H
#include <functional>
#include "config.hpp"


namespace integration {
	template<typename Body>
	using IntegrationMethod = std::function<void(Body&, double, const typename Body::Vector&)>;

	template<typename Body>
	void euler(Body& body, double dt, const typename Body::Vector& acc) {
		body.vel += acc * dt;
		body.pos += body.vel * dt;
	}

	template<typename Body>
	void leapfrog(Body& body, double dt, const typename Body::Vector& acc) {
		auto nextvel = body.vel + acc*dt*0.5;
		body.pos += nextvel*dt*0.5;
		body.vel = nextvel;
	}

	template<typename Body>
	IntegrationMethod<Body> get(config::Config cfg) {
		auto name = cfg.get_or_fail<std::string>("simulation.integration.type");

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