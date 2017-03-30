#ifndef GLARE_ERROR_HPP
#define GLARE_ERROR_HPP

#include <stdexcept>

namespace Glare {
	namespace Error {
		class Glare_error : public std::runtime_error {
		public:
			Glare_error(std::string s) :std::runtime_error{s}{};
		};

		class Slot_map_pointer_not_valid : public Glare_error {
		public:
			Slot_map_pointer_not_valid(std::string s) :Glare_error {s}{};
		};

		class Slot_map_iterator_out_of_range : public Glare_error {
		public:
			Slot_map_iterator_out_of_range(std::string s) :Glare_error {s}{};
		};
	}
}

#endif // !GLARE_ERROR_HPP
