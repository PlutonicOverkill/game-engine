#ifndef ERROR_HPP
#define ERROR_HPP

#include <stdexcept>

namespace Engine
{
	class Engine_error : public std::runtime_error {
		using std::runtime_error::runtime_error; // inherit constructors
	};
}

#endif // ERROR_HPP
