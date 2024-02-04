#ifndef CONFIG_H
#define CONFIG_H

#include <toml++/toml.hpp>
#include <unordered_map>
#include <ranges>
#include <cmath>
#include "spatial.hpp"


namespace config {
	class configuration_error : public std::runtime_error
	{
	public:
		configuration_error(const std::string& msg): std::runtime_error(msg) {};
		configuration_error(std::string&& msg): std::runtime_error(msg) {};
	};

	void backend_fail(const std::string& backend) {
		throw configuration_error("Unable to select " + backend + " backend.");
	}

	class Config {
	private:
		toml::table* tbl_;
		
	public:
		Config(toml::table* tbl): tbl_(tbl) {}
		Config(toml::table& tbl) {
			if (!tbl.is_table()) {
				throw configuration_error("Incorrect configuration.");
			}
			tbl_ = tbl.as_table();
		}

		std::optional<Config> get(const std::string& path) {
			auto c = tbl_->at_path(path);
			if (!c.is_table()) {
				return {};
			}
			return Config(c.as_table());
		}

		template<typename T>
		std::optional<T> get(const std::string& path) {
			return tbl_->at_path(path).value<T>();
		}

		template<typename T>
		T get_or_fail(const std::string& path) {
			std::optional<T> opt = get<T>(path);
			if (!opt.has_value())
				throw config::configuration_error("Required key '" + path + "' not found in configuration.");
			return *opt;
		}

		Config get_or_fail(const std::string& path) {
			std::optional<Config> opt = get(path);
			if (!opt.has_value())
				throw config::configuration_error("Required key '" + path + "' not found in configuration.");
			return *opt;
		}
	};

	template<typename T, spatial::Dimension D>
	struct get_coords_or_fail;

	template<typename T>
	struct get_coords_or_fail<T, 2> {
		std::array<T, 2> operator()(config::Config cfg, const std::string& path) {
			return {
				cfg.get_or_fail<T>(path + ".x"),
				cfg.get_or_fail<T>(path + ".y")
			};
		}
	};

	template<typename T>
	struct get_coords_or_fail<T, 3> {
		std::array<T, 3> operator()(config::Config cfg, const std::string& path) {
			return {
				cfg.get_or_fail<T>(path + ".x"),
				cfg.get_or_fail<T>(path + ".y"),
				cfg.get_or_fail<T>(path + ".z")
			};
		}
	};

	class ConfigurationManager {
	private:
		toml::parse_result data;
	public:
		ConfigurationManager(const std::string& path) {
			data = toml::parse_file(path);
		}

		Config get_config() {
			return Config(data);
		}
	};

	class Units {
	public:
		enum class Quantity {
			DIST,
			TIME,
			MASS,
		};
		static constexpr std::array<Quantity, 3> quantities = {
			Quantity::DIST,
			Quantity::TIME,
			Quantity::MASS
		};

		struct SimulationUnit {
			std::string unit;
			double value;

			double si_value;

			std::string to_string() const {
				return std::to_string(value) + " " + unit;
			}
		};

		double G0;

	private:
		std::unordered_map<Quantity, SimulationUnit> units;

		static std::optional<double> si_prefix(std::string_view unit) {
			static constexpr std::array<const char*, 24> prefixes  = {"Q","R","Y","Z","E","P","T","G","M","k","h","da","d","c","m","μ","n","p","f","a","z","y","r","q"};
			static constexpr std::array<double,      24> exponents = { 30, 27, 24, 21, 18, 15, 12,  9,  6,  3,  2,   1, -1, -2, -3, -6, -9,-12,-15,-18,-21,-24,-27,-30};

			for (auto&& [pref, ex] : std::ranges::views::zip(prefixes, exponents)) {
				if (unit == pref) {
					return ex;
				}
			}
			return {};
		}

		static std::optional<double> to_base_units(std::string_view unit) {
			static constexpr std::array<std::string_view, 6> units = {
				"m",
				"s",
				"g",
				"pc",
				"year",
				"mass_sun",
			};
			static constexpr std::array<double, 6> values = {
				1,
				1,
				1e-3,
				30856775810000000.,
				60*60*24*365.,
				1.989e30,
			};

			for (auto&& [u, v] : std::ranges::views::zip(units, values)) {
				if (unit.ends_with(u)) {
					auto prefix = unit.substr(0, unit.size()-u.size());
					if (prefix.empty()) {
						return v;
					}
					auto ex = si_prefix(prefix);
					if (!ex.has_value()) {
						return {};
					}
					return v * std::pow(10, *ex);
				}
			}
			return {};
		}

		static std::optional<SimulationUnit> get_cfg_unit(Config cfg) {
			return cfg.get<std::string>("unit").and_then([&cfg](std::string&& unit) {
				return to_base_units(unit).transform([unit = std::move(unit), &cfg](double base_unit) {
					auto val = cfg.get<double>("val").value_or(1.);

					return SimulationUnit {
						unit, 
						val,
						base_unit*val
					};
				});
			});
		}

		template<typename T>
		static T unwrap(std::optional<T> opt, const std::string& msg) {
			if (!opt.has_value()) {
				throw configuration_error("Invalid configuration. Incorrect/missing " + msg + ".");
			}
			return *opt;
		}

	public:
		Units(Config cfg) {
			G0 = cfg.get_or_fail<double>("physical.G0");

			static std::array<std::string, 3> quantity_keys = {
				"dist", "time", "mass"
			};

			auto units_cfg = cfg.get_or_fail("simulation.units");
			for (std::size_t i = 0; i < quantities.size(); ++i) {
				auto unit = unwrap(get_cfg_unit(units_cfg.get_or_fail(quantity_keys[i])), std::string(quantity_keys[i]) + " unit specification");
				units.emplace(quantities[i], std::move(unit));
			}
		}

		SimulationUnit unit(Quantity q) const {
			auto it = units.find(q);
			if (it != units.end()) {
				return it->second;
			} else {
				throw configuration_error("Improperly configured units.");
			}
		}

		double base_unit(Quantity q) const {
			return unit(q).si_value;
		}

		double G() const {
			auto dist_unit = base_unit(Quantity::DIST);
			auto time_unit = base_unit(Quantity::TIME);
			auto mass_unit = base_unit(Quantity::MASS);
			return G0 * (time_unit*time_unit) / (dist_unit*dist_unit*dist_unit) * mass_unit;
		}
	};

	inline std::ostream& operator<<(std::ostream &os, const Units::SimulationUnit& unit) { 
		return os << unit.to_string();
	}
}


#endif