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
				template<typename U>
				using Ptr_type = typename Slot_map<U>::template Index_base<Is_const>;
				// safe pointer to each component type
				std::tuple<Ptr_type<T>...> ptr;
			}; // Entity_base

			using Entity = typename Entity_base<false>;
			using Const_entity = typename Entity_base<true>;

			template<bool Is_const>
			class Index_base {
			public:

			private:
				typename Slot_map<Entity>::template Index_base<Is_const> ptr;
			};

			using Stable_index = typename Index_base<false>;
			using Stable_const_index = typename Index_base<true>;

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

			Stable_index add();

			template<bool Is_const, typename... U>
			bool has_component(Index_base<Is_const>) const;

			// TODO: variadic overloads
			template<typename U>
			const U& component(Stable_const_index) const;
			template<typename U>
			U& component(Stable_index);

			// TODO: variadic overloads
			template<typename U>
			U& make_component(Stable_index);

			// TODO: variadic overloads
			template<typename U>
			U* check_component(Stable_const_index) const;
			template<typename U>
			const U* check_component(Stable_index);
		private:
			template<bool Is_const, typename U, typename... V>
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

template<typename ...T>
template<typename ...U>
template<bool Is_const>
typename Glare::Ecs::Entity_manager<T...>::Entity_base<Is_const>
Glare::Ecs::Entity_manager<T...>::Range<U...>::Iterator_base<Is_const>::operator*()
{
	return *iter;
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

template<typename... T>
template<bool Is_const, typename... U>
bool Glare::Ecs::Entity_manager<T...>::has_component
(typename Glare::Ecs::Entity_manager<T...>::Index_base<Is_const> e) const
{
	return has_component<Is_const, U...>(ents[e]);
}

template<typename... T>
template<typename U>
const U& Glare::Ecs::Entity_manager<T...>::component
(typename Glare::Ecs::Entity_manager<T...>::Stable_const_index e) const
{
	return component(ents[e]);
}

template<typename... T>
template<typename U>
U& Glare::Ecs::Entity_manager<T...>::component
(typename Glare::Ecs::Entity_manager<T...>::Stable_index e)
{
	return component(ents[e]);
}

template<typename... T>
template<bool Is_const, typename U, typename... V>
bool Glare::Ecs::Entity_manager<T...>::has_component
(typename Glare::Ecs::Entity_manager<T...>::Entity_base<Is_const> e) const
{
	return std::get<U>(components).is_valid
		(std::get<Glare::Slot_map<U>::Stable_index>(e))
		&& (sizeof...(V) > 0) ? has_component<Is_const, V...>(e) : true;
}

template<typename... T>
template<typename U>
const U& Glare::Ecs::Entity_manager<T...>::component
(typename Glare::Ecs::Entity_manager<T...>::Const_entity e) const
{
	return std::get<U>(components)[std::get<Slot_map<U>::Stable_index>(e)];
}

template<typename... T>
template<typename U>
U& Glare::Ecs::Entity_manager<T...>::component
(typename Glare::Ecs::Entity_manager<T...>::Entity e)
{
	return std::get<U>(components)[std::get<Slot_map<U>::Stable_index>(e)];
}

template<typename... T>
template<typename U>
U& Glare::Ecs::Entity_manager<T...>::make_component
(typename Glare::Ecs::Entity_manager<T...>::Entity e)
{
	if (!std::get<U>(components)
		.is_valid(std::get<Glare::Slot_map<U>::Stable_index>(e)))
	{
		std::get<Glare::Slot_map<U>::Stable_index>(e) = std::get<U>(components).add();
	}
	return std::get<U>(components)[std::get<Slot_map<U>::Stable_index>(e)];
}

template<typename... T>
template<typename U>
U* Glare::Ecs::Entity_manager<T...>::check_component
(typename Glare::Ecs::Entity_manager<T...>::Const_entity) const
{
	if (std::get<U>(components)
		.is_valid(std::get<Glare::Slot_map<U>::Stable_index>(e)))
	{
		return &(std::get<U>(components)[std::get<Glare::Slot_map<U>::Stable_index>(e)]);
	}
	else {
		return nullptr;
	}
}

#endif // !GLARE_ECS_HPP
