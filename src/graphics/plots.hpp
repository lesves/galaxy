#ifndef GALAXY_PLOTS_H
#define GALAXY_PLOTS_H

#include <vector>
#include "../config.hpp"

#ifdef USE_OPENCV_GRAPHICS
	#include "opencv/plots_impl.hpp"
#else
	#include "raylib/plots_impl.hpp"
#endif


namespace plots {
	class LinearStatsPlot {
	private:
		std::size_t plot_height;
		std::size_t plot_width;

		plots::PlotWindow win;

	public:
		virtual std::string name() = 0;

		virtual std::size_t size() = 0;
		virtual bool empty() = 0;
		virtual double operator[](std::size_t idx) = 0;

		virtual ~LinearStatsPlot() {};

		LinearStatsPlot(std::size_t plot_width, std::size_t plot_height): plot_height(plot_height), plot_width(plot_width), win(plot_width, plot_height) {};

		void show() {
			win.set_name(name());
			win.begin_plot();

			if (size() < 2) {
				return;
			}

			win.line(0, plot_height/2, plot_width, plot_height/2, plots::color(0, 255, 0));

			auto base = (*this)[0];
			auto end = size();
			auto start = end >= plot_width ? end-plot_width : 0;

			for (std::size_t i = 1; i < (end >= plot_width ? plot_width : end); ++i) {
				win.line(
					i-1, plot_height - ((*this)[start+i-1])/(base*2)*plot_height, 
					i,   plot_height - ((*this)[start+i  ])/(base*2)*plot_height, 
					plots::color(255, 255, 255)
				);
			}

			win.end_plot();
		}
	};

	class EnergyStatsPlot : public LinearStatsPlot {
		std::vector<double> kin_energy_;
		std::vector<double> pot_energy_;

	public:
		EnergyStatsPlot(config::Config cfg): LinearStatsPlot(
			cfg.get_or_fail<double>("simulation.plots.energy.size.width"),
			cfg.get_or_fail<double>("simulation.plots.energy.size.height")
		) {}

		virtual std::string name() override {
			return "energy";
		}

		virtual std::size_t size() override {
			return kin_energy_.size();
		}

		virtual bool empty() override {
			return kin_energy_.empty();
		}

		virtual double operator[](std::size_t idx) override {
			return kin_energy_[idx] + pot_energy_[idx];
		}

		virtual void log(double kin, double pot) {
			kin_energy_.push_back(kin);
			pot_energy_.push_back(pot);
		}
	};
}

#endif