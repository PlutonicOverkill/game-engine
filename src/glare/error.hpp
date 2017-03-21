#ifndef GLARE_ERROR_HPP
#define GLARE_ERROR_HPP

#include <stdexcept>

namespace Glare {
	namespace Error {
		class Glare_error : public std::runtime_error {
		public:
			Glare_error(std::string s) :std::runtime_error{s}{};
		};
	}
}

#endif // !GLARE_ERROR_HPP
