#include "orthtree.hpp"


namespace sim {
	template<typename Policy>
	class TreeSimulation {
	private:
		Policy policy_;

	public:
		std::vector<orthtree::Point<Policy::NumType, Policy::Dim>> bodies;

		TreeSimulation(const Policy& policy): policy_(policy), bodies(policy.mass_distribution()) {}
		TreeSimulation(): bodies(policy_.mass_distribution()) {}
		
		void step() {
			OrthTree<Policy::NumType, Policy::Dim, Policy> tree(policy_, policy_.bbox);
			for (auto&& body : bodies) {
				tree.insert(body);
			}
		}
	};
}