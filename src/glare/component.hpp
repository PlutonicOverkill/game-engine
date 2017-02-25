#ifndef GLARE_COMPONENT_HPP
#define GLARE_COMPONENT_HPP

#include <string>
#include <memory>
#include <utility>

namespace Glare {
/*
	Entities are made up of multiple components
*/
	struct Component {
		// copying disabled to prevent slicing
		Component(const Component&) = delete;
		Component& operator=(const Component&) = delete;

		virtual std::string name() = 0;

		virtual ~Component();
	};

	using Component_ptr = std::unique_ptr<Component>;

	/*
		Perfect forwarding FTW
	*/
	template<typename T, typename... Args>
	Component_ptr make_component(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}

#endif // !GLARE_COMPONENT_HPP
