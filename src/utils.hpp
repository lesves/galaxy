#ifndef GALAXY_UTILS_H
#define GALAXY_UTILS_H
#include <iomanip>
#include <sstream>

template<typename F>
std::string formatf(F num, std::size_t prec) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(prec) << num;
	return stream.str();
}

#endif