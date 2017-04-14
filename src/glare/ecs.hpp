#ifndef GLARE_ECS_HPP
#define GLARE_ECS_HPP

#include "utility.hpp"
#include "slot_map.hpp"

#include <tuple>
#include <utility>

namespace Glare {
	// Entity component system
	namespace Ecs {
		namespace Impl {
			template<typename... Ts>
			struct Typelist {};

			template<template<typename> class Cont, typename... Args>
			[[maybe_unused]] std::tuple<Cont<Args>...> typelist_helper(Typelist<Args...>);

			template<template<typename> class Cont, typename T>
			using Variadic_cont = decltype(typelist_helper<Cont>(std::declval<T>()));

			template<typename T>
			struct Indexed_element {
				// TODO
			};

			template<typename T>
			using Slot_pointer = typename Slot_map<T>::pointer;
		}

		// a manager class, how original
		// T is a list of all the component types usable by Entities
		template<typename... T>
		class Entity_manager {
		public:
			Entity_manager() = default;

			class Entity {
			public:

			private:
				// safe pointer to each component type
				std::tuple<Impl::Slot_pointer<T>...> ptr;
			};
		private:
			// Impl::Variadic_cont<Slot_map, Impl::Typelist<Impl::Indexed_element<T>...>> vec;

			Slot_map<std::tuple<Impl::Slot_pointer<T>...>> ents;
		};
	}
}

#endif // !GLARE_ECS_HPP
