#ifndef GLARE_FACTORY_HPP
#define GLARE_FACTORY_HPP

#include <unordered_map>
#include <string>
#include <type_traits>

namespace Glare {
	template<typename Base>
	class Factory {
	public:
		template<typename T>
		void register_type(std::string); // I wanted to call this register but that's a keyword

		Base* create(std::string name);
	private:
		template<typename T>
		static Base* create_instance();

		using ctor_fn = Base* (*)(); // no args for now
		std::unordered_map<std::string, ctor_fn> ctors;
	};
}

template<typename Base>
template<typename T>
void Factory<Base>::register_type(std::string name)
{
	static_assert(std::experimental::is_base_of_v<Base, T>);
	ctors[name] = &create_instance<T>;
}

template<typename Base>
Base* Factory<Base>::create(std::string name)
{
	auto it = ctors.find(name);
	if(it != ctors.end()) {
		return it->second();
	} else {
		return nullptr;
	}
}

template<typename Base>
template<typename T>
static Base* create_instance()
{
	return new T{};
}

#endif // !GLARE_FACTORY_HPP
