#ifndef GALAXY_PLOTS_H
#define GALAXY_PLOTS_H

#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "config.hpp"


namespace plots {
	class LinearStatsPlot {
	private:
		std::size_t plot_height;
		std::size_t plot_width;

	public:
		virtual std::string name() = 0;

		virtual std::size_t size() = 0;
		virtual bool empty() = 0;
		virtual double operator[](std::size_t idx) = 0;

		virtual ~LinearStatsPlot() {};

		LinearStatsPlot(std::size_t plot_height, std::size_t plot_width): plot_height(plot_height), plot_width(plot_width) {}

		void show() {
			if (size() < 2) {
				return;
			}

			cv::Mat plot_img(plot_height, plot_width, CV_8UC3, cv::Scalar(0, 0, 0));

			cv::line(plot_img, cv::Point(0, plot_height/2), cv::Point(plot_width, plot_height/2), cv::Scalar(0, 255, 0));

			auto base = (*this)[0];

			auto end = size();
			auto start = end >= plot_width ? end-plot_width : 0;

			for (std::size_t i = 1; i < (end >= plot_width ? plot_width : end); ++i) {
				cv::line(
					plot_img, 
					cv::Point(i-1, plot_height - ((*this)[start+i-1])/(base*2)*plot_height), 
					cv::Point(i,   plot_height - ((*this)[start+i  ])/(base*2)*plot_height), 
					cv::Scalar(255, 255, 255)
				);
			}

			cv::imshow(name(), plot_img);
		}
	};

	class EnergyStatsPlot : public LinearStatsPlot {
		std::vector<double> kin_energy_;
		std::vector<double> pot_energy_;

	public:
		EnergyStatsPlot(config::Config cfg): LinearStatsPlot(
				cfg.get_or_fail<double>("simulation.plots.energy.size.height"),
				cfg.get_or_fail<double>("simulation.plots.energy.size.width")
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