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

			// purposely not defined
			template<template<typename> class Cont, typename... Args>
			[[maybe_unused]] std::tuple<Cont<Args>...> typelist_helper(Typelist<Args...>);

			template<template<typename> class Cont, typename T>
			using Variadic_cont = decltype(typelist_helper<Cont>(std::declval<T>()));
		}

		// Slot_map<T0>::Index_base<Is_const>
		// Slot_map<T1>::Index_base<Is_const>
		// ...
		// Slot_map<TN>::Index_base<Is_const>

		// std::tuple<Index_base<Slot_map<T>...>> ptr

		// a manager class, how original
		// T is a list of all the component types usable by Entities
		template<typename... T>
		class Entity_manager {
		public:
			Entity_manager() = default;

			template<bool Is_const>
			class Entity_base {
			public:

			private:
				// safe pointer to each component type
				// std::tuple<> ptr;
			}; // Entity_base

			using Entity = typename Entity_base<false>;
			using Const_entity = typename Entity_base<true>;

			template<typename... U>
			class Range {
			public:
				explicit Range(Entity_manager*);

				// classception
				template<bool Is_const>
				class Iterator_base {
				public:
					Iterator_base(Entity_manager*,
								  typename Slot_map<Entity>::template Iterator_base<Is_const>,
								  typename Slot_map<Entity>::template Iterator_base<Is_const>);

					Iterator_base& operator++();

					template<bool V>
					bool operator==(Iterator_base<V>) const;
					template<bool V>
					bool operator!=(Iterator_base<V>) const;

					Entity_base<Is_const> operator*();
				private:
					Entity_manager* ptr;

					typename Slot_map<Entity>::template Iterator_base<Is_const> iter;
					typename Slot_map<Entity>::template Iterator_base<Is_const> end;
				}; // iterator_base

				using iterator = Iterator_base<false>;
				using const_iterator = Iterator_base<true>;

				iterator begin();
				const_iterator begin() const;
				const_iterator cbegin() const;

				iterator end();
				const_iterator end() const;
				const_iterator cend() const;
			private:
				Entity_manager* ptr;
			}; // Range

			template<typename... U>
			Range<U...> filter();

			Entity add();

			template<bool Is_const, typename U, typename... V>
			bool has_component(Entity_base<Is_const>) const;
			template<bool Is_const, typename U>
			bool has_component(Entity_base<Is_const>) const;

			// TODO: variadic overloads
			template<typename U>
			const U& component(Const_entity) const;
			template<typename U>
			U& component(Entity);

			// TODO: variadic overloads
			template<typename U>
			U& make_component(Entity);

			// TODO: variadic overloads
			template<typename U>
			U* check_component(Const_entity) const;
			template<typename U>
			const U* check_component(Entity);
		private:
			// only a nested class to avoid cyclic dependencies
			template<typename U>
			struct Indexed_element {
				U val;
				typename Slot_map<Entity>::Stable_index index;
			};

			Impl::Variadic_cont<Slot_map, Impl::Typelist<Indexed_element<T>...>> components;

			Slot_map<Entity> ents;
		}; // Entity_manager
	}
}

/***** IMPLEMENTATION *****/

template<typename... T>
template<typename... U>
Glare::Ecs::Entity_manager<T...>::Range<U...>::Range
(Glare::Ecs::Entity_manager<T...>* ptr)
	:ptr {ptr}
{}

template<typename... T>
template<typename... U>
Glare::Ecs::Entity_manager<T...>::Range<U...> Glare::Ecs::Entity_manager<T...>::filter()
{
	return {this};
}

template<typename... T>
template<typename... U>
template<bool Is_const>
Glare::Ecs::Entity_manager<T...>::Range<U...>::Iterator_base<Is_const>::Iterator_base
(Glare::Ecs::Entity_manager<T...>* ptr,
 typename Slot_map<Entity>::template Iterator_base<Is_const> iter,
 typename Slot_map<Entity>::template Iterator_base<Is_const> end)
	:ptr {ptr},
	iter {iter},
	end {end}
{}

template<typename... T>
template<typename... U>
template<bool Is_const>
Glare::Ecs::Entity_manager<T...>::Range<U...>::Iterator_base<Is_const>&
Glare::Ecs::Entity_manager<T...>::Range<U...>::Iterator_base<Is_const>::operator++()
{
	do {
		++iter;
	} while (iter != end && !ptr->has_component<U...>(*iter));
	return *this;
}

template<typename... T>
template<typename... U>
template<bool Is_const>
template<bool V>
bool Glare::Ecs::Entity_manager<T...>::Range<U...>::Iterator_base<Is_const>::operator==(
	Glare::Ecs::Entity_manager<T...>::Range<U...>::Iterator_base<V> rhs) const
{
	// probably don't need to compare pointers
	return iter == rhs.iter;
}

template<typename... T>
template<typename... U>
template<bool Is_const>
template<bool V>
bool Glare::Ecs::Entity_manager<T...>::Range<U...>::Iterator_base<Is_const>::operator!=(
	Glare::Ecs::Entity_manager<T...>::Range<U...>::Iterator_base<V> rhs) const
{
	// probably don't need to compare pointers
	return (*this) != rhs;
}

template<typename... T>
template<typename... U>
typename Glare::Ecs::Entity_manager<T...>::Range<U...>::iterator
Glare::Ecs::Entity_manager<T...>::Range<U...>::begin()
{
	return {ptr, ptr->ents.begin(), ptr->ents.end()};
}

template<typename... T>
template<typename... U>
typename Glare::Ecs::Entity_manager<T...>::Range<U...>::const_iterator
Glare::Ecs::Entity_manager<T...>::Range<U...>::begin() const
{
	return {ptr, ptr->ents.cbegin(), ptr->ents.cend()};
}

template<typename... T>
template<typename... U>
typename Glare::Ecs::Entity_manager<T...>::Range<U...>::const_iterator
Glare::Ecs::Entity_manager<T...>::Range<U...>::cbegin() const
{
	return {ptr, ptr->ents.cbegin(), ptr->ents.cend()};
}

template<typename... T>
template<typename... U>
typename Glare::Ecs::Entity_manager<T...>::Range<U...>::iterator
Glare::Ecs::Entity_manager<T...>::Range<U...>::end()
{
	return {ptr, ptr->ents.end(), ptr->ents.end()};
}

template<typename... T>
template<typename... U>
typename Glare::Ecs::Entity_manager<T...>::Range<U...>::const_iterator
Glare::Ecs::Entity_manager<T...>::Range<U...>::end() const
{
	return {ptr, ptr->ents.cend(), ptr->ents.cend()};
}

template<typename... T>
template<typename... U>
typename Glare::Ecs::Entity_manager<T...>::Range<U...>::const_iterator
Glare::Ecs::Entity_manager<T...>::Range<U...>::cend() const
{
	return {ptr, ptr->ents.cend(), ptr->ents.cend()};
}

#endif // !GLARE_ECS_HPP
