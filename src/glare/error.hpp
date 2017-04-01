#ifndef GLARE_ERROR_HPP
#define GLARE_ERROR_HPP

#include <stdexcept>
#include <utility>

namespace Glare {
	namespace Error {
		class Glare_error : public std::runtime_error {
		public:
			Glare_error(std::string s) :std::runtime_error{std::move(s)}{};
		};

		class Slot_map_pointer_not_valid : public Glare_error {
		public:
			Slot_map_pointer_not_valid(std::string s) :Glare_error {std::move(s)}{};
		};

		class Slot_map_out_of_range : public Glare_error {
			Slot_map_out_of_range(std::string s) :Glare_error {std::move(s)}{};
		};
	}
}

#endif // !GLARE_ERROR_HPP
