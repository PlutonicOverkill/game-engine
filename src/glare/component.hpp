#ifndef GLARE_COMPONENT_HPP
#define GLARE_COMPONENT_HPP

#include "factory.hpp"

#include <string>
#include <memory>
#include <iostream>
#include <utility>
#include <unordered_map>

namespace Glare {
/*
	Entities are made up of multiple components
*/
	struct Component {
		// disabled to prevent slicing
		Component(const Component&) = delete;
		Component& operator=(const Component&) = delete;

		Component(Component&&) = delete;
		Component& operator=(Component&&) = delete;

		// virtual Component* clone() = 0;

		virtual ~Component() {};
	};

	using Component_ptr = std::unique_ptr<Component>;


	/*
		Perfect forwarding FTW
	*/
	template<typename T, typename... Args>
	Component_ptr make_component(Args&&... args);
}

template<typename T, typename... Args>
Glare::Component_ptr Glare::make_component(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

#endif // !GLARE_COMPONENT_HPP
