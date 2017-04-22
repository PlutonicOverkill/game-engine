#ifndef GLARE_ECS_HPP
#define GLARE_ECS_HPP

#include "utility.hpp"
#include "slot_map.hpp"

#include <tuple>
#include <utility>
#include <type_traits>

namespace Glare {
	namespace Ecs_impl {
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
		struct Entity;

		// only a nested class to avoid cyclic dependencies
		template<typename U>
		struct Indexed_element {
			U val;
			typename Slot_map<Entity>::Stable_index index;
		};

		struct Entity {
			template<typename U>
			using Elem_type = typename Slot_map<Indexed_element<U>>::Stable_index;
			using Ptr_type = std::tuple<Elem_type<T>...>;

			// safe pointer to each component type
			typename Ptr_type ptr;
		}; // Entity
	public:
		Entity_manager() = default;

		template<bool Is_const>
		using Index_base = typename Slot_map<Entity>::template Index_base<Is_const>;
		using Stable_index = typename Slot_map<Entity>::template Index_base<false>;
		using Stable_const_index = typename Slot_map<Entity>::template Index_base<true>;

		// TODO: should have the same interface as Iterator_base
		template<bool Is_const, typename U>
		class Component_iterator_base {
		public:
			Component_iterator_base
			(std::conditional_t<Is_const,
				const Entity_manager*, Entity_manager*> ptr,
			typename Slot_map<Indexed_element<U>>::template Iterator_base<Is_const>);

			template<typename... U>
			bool has_component() const;
		private:
			template<typename First, typename... Rest,
				typename std::enable_if_t<std::is_same_v<First, U>, int> = 0>
			bool has_component_impl() const;
			template<typename First, typename... Rest,
				typename std::enable_if_t<!std::is_same_v<First, U>, int> = 0>
			bool has_component_impl() const;
			template<typename... Rest,
				typename = typename std::enable_if_t<sizeof...(Rest) == 0>>
			bool has_component_impl() const;

			friend class Entity_manager;
			std::conditional_t<Is_const,
				const Entity_manager*, Entity_manager*> ptr;
			typename Slot_map<Indexed_element<U>>::template Iterator_base<Is_const> iter;
		};

		template<bool Is_const>
		class Entity_iterator_base {
		public:
			Entity_iterator_base
			(std::conditional_t<Is_const,
				const Entity_manager*, Entity_manager*> ptr,
			 typename Slot_map<Entity>::template Iterator_base<Is_const>);
		private:
			friend class Entity_manager;
			std::conditional_t<Is_const,
				const Entity_manager*, Entity_manager*> ptr;
			typename Slot_map<Entity>::template Iterator_base<Is_const> iter;
		};

		template<typename U>
		using Component_iterator = typename Component_iterator_base<false, U>;
		template<typename U>
		using Component_iterator_const = typename Component_iterator_base<true, U>;

		using Entity_iterator = typename Entity_iterator_base<false>;
		using Entity_iterator_const = typename Entity_iterator_base<true>;

		template<bool Is_const, typename... U>
		class Range_base {
		public:
			explicit Range_base(std::conditional_t<Is_const,
				const Entity_manager*, Entity_manager*>);

			template<bool Iter_const>
			class Iterator_base {
			public:
				Iterator_base(std::conditional_t<Iter_const,
							  const Entity_manager*, Entity_manager*>,
					typename Slot_map<Entity>::template Iterator_base<Iter_const>,
					typename Slot_map<Entity>::template Iterator_base<Iter_const>);

				Iterator_base& operator++();

				template<bool V>
				bool operator==(Iterator_base<V>) const;
				template<bool V>
				bool operator!=(Iterator_base<V>) const;

				Entity_iterator_base<Iter_const> operator*();
			private:
				std::conditional_t<Iter_const,
					const Entity_manager*, Entity_manager*> ptr;

				typename Slot_map<Entity>::template Iterator_base<Iter_const> iter;
				typename Slot_map<Entity>::template Iterator_base<Iter_const> end;
			}; // iterator_base

			using iterator = Iterator_base<false>;
			using const_iterator = Iterator_base<true>;

			std::conditional_t<Is_const, const_iterator, iterator> begin() const;
			const_iterator cbegin() const;

			std::conditional_t<Is_const, const_iterator, iterator> end() const;
			const_iterator cend() const;
		private:
			std::conditional_t<Is_const,
				const Entity_manager*, Entity_manager*> ptr;
		}; // Range_base

		template<bool Is_const, typename U>
		class Component_range_base {
		public:
			explicit Component_range_base(std::conditional_t<Is_const,
				const Entity_manager*, Entity_manager*>);

			template<bool Iter_const>
			class Iterator_base {
			public:
				Iterator_base
					(std::conditional_t<Iter_const,
						const Entity_manager*, Entity_manager*>,
					typename Slot_map<Indexed_element<U>>::template Iterator_base<Iter_const>);

				Iterator_base& operator++();

				template<bool V>
				bool operator==(Iterator_base<V>) const;
				template<bool V>
				bool operator!=(Iterator_base<V>) const;

				Component_iterator_base<Iter_const, U> operator*();
			private:
				std::conditional_t<Iter_const,
					const Entity_manager*, Entity_manager*> ptr;
				typename Slot_map<Indexed_element<U>>::template Iterator_base<Iter_const> iter;
			}; // iterator_base

			using iterator = Iterator_base<false>;
			using const_iterator = Iterator_base<true>;

			std::conditional_t<Is_const, const_iterator, iterator> begin() const;
			const_iterator cbegin() const;

			std::conditional_t<Is_const, const_iterator, iterator> end() const;
			const_iterator cend() const;
		private:
			std::conditional_t<Is_const,
				const Entity_manager*, Entity_manager*> ptr;
		}; // Component_range_base

		template<typename... U>
		using Range = typename Range_base<false, U...>;
		template<typename... U>
		using Range_const = typename Range_base<true, U...>;
		template<typename U>
		using Component_range = typename Component_range_base<false, U>;
		template<typename U>
		using Component_range_const = typename Component_range_base<true, U>;

		template<typename... U, typename std::enable_if_t<(sizeof...(U) > 1), int> = 0>
		Range<U...> filter(); // more than one element
		template<typename U>
		Component_range<U> filter(); // single element
		Range<> filter(); // no elements

		// const versions
		template<typename... U, typename std::enable_if_t<(sizeof...(U) > 1), int> = 0>
		Range_const<U...> filter() const; // more than one element
		template<typename U>
		Component_range_const<U> filter() const; // single element
		Range_const<> filter() const; // no elements

		template<typename... U, typename std::enable_if_t<(sizeof...(U) > 1), int> = 0>
		Range_const<U...> cfilter() const; // more than one element
		template<typename U>
		Component_range_const<U> cfilter() const; // single element
		Range_const<> cfilter() const; // no elements

		Stable_index add();
		// TODO: buffered_add
		// TODO: remove
		// TODO: buffered_remove
		// TODO: emplace_component
		// TODO: remove_component
		// TODO: buffered_add_component
		// TODO: buffered_remove_component

		template<typename... U>
		bool has_component(Stable_const_index) const;
		template<typename... U>
		bool has_component(Stable_index) const;

		// TODO: variadic overloads
		template<typename U>
		const U& component(Stable_const_index) const;
		template<typename U>
		U& component(Stable_index);

		// TODO: variadic overloads
		template<typename U, typename V>
		const U& component(Component_iterator_const<V>) const;
		template<typename U, typename V>
		U& component(Component_iterator<V>);

		// TODO: variadic overloads
		template<typename U>
		U& make_component(Stable_index);

		// TODO: variadic overloads
		template<typename U, typename V>
		U& make_component(Component_iterator<V>);

		// TODO: variadic overloads
		template<typename U>
		const U* check_component(Stable_const_index) const;
		template<typename U>
		U* check_component(Stable_index);

		// TODO: variadic overloads
		template<typename U, typename V>
		const U* check_component(Component_iterator_const<V>) const;
		template<typename U, typename V>
		U* check_component(Component_iterator<V>);
	private:
		template<typename First, typename... Rest>
		bool has_component_impl(Stable_const_index) const;
		template<typename... Rest,
			typename = typename std::enable_if_t<sizeof...(Rest) == 0>>
		bool has_component_impl(Stable_const_index) const;

		Ecs_impl::Variadic_cont<Slot_map, Ecs_impl::Typelist<Indexed_element<T>...>> components;

		Slot_map<Entity> ents;
	}; // Entity_manager
}


/***** IMPLEMENTATION *****/

template<typename... T>
template<bool Is_const, typename U>
Glare::Entity_manager<T...>::Component_iterator_base<Is_const, U>::Component_iterator_base
(std::conditional_t<Is_const,
	const Glare::Entity_manager<T...>*, Glare::Entity_manager<T...>*> ptr,
typename Glare::Slot_map<typename Glare::Entity_manager<T...>::Indexed_element<U>>::template
Iterator_base<Is_const> iter)
	:ptr {ptr},
	iter {iter}
{}

template<typename... T>
template<bool Is_const>
Glare::Entity_manager<T...>::Entity_iterator_base<Is_const>::Entity_iterator_base
(std::conditional_t<Is_const,
	const Glare::Entity_manager<T...>*, Glare::Entity_manager<T...>*> ptr,
 typename Glare::Slot_map<typename Glare::Entity_manager<T...>::Entity>::template
 Iterator_base<Is_const> iter)
	:ptr {ptr},
	iter {iter}
{}

template<typename... T>
template<bool Is_const, typename... U>
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Range_base
(std::conditional_t<Is_const,
	const Entity_manager*, Entity_manager*> ptr)
	:ptr {ptr}
{}

template<typename... T>
template<bool Is_const, typename U>
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Component_range_base
(std::conditional_t<Is_const,
	const Entity_manager*, Entity_manager*> ptr)
	: ptr {ptr}
{}

template<typename... T>
template<typename... U, typename std::enable_if_t<(sizeof...(U) > 1), int>>
typename Glare::Entity_manager<T...>::Range<U...>
Glare::Entity_manager<T...>::filter()
{
	return Glare::Entity_manager<T...>::Range<U...>{this};
}

template<typename... T>
template<typename U>
typename Glare::Entity_manager<T...>::Component_range<U>
Glare::Entity_manager<T...>::filter()
{
	return Glare::Entity_manager<T...>::Component_range<U>{this};
}

template<typename... T>
typename Glare::Entity_manager<T...>::Range<>
Glare::Entity_manager<T...>::filter()
{
	return Glare::Entity_manager<T...>::Range<>{this};
}

template<typename... T>
template<typename... U, typename std::enable_if_t<(sizeof...(U) > 1), int>>
typename Glare::Entity_manager<T...>::Range_const<U...>
Glare::Entity_manager<T...>::cfilter() const
{
	return Glare::Entity_manager<T...>::Range_const<U...>{this};
}

template<typename... T>
template<typename U>
typename Glare::Entity_manager<T...>::Component_range_const<U>
	Glare::Entity_manager<T...>::cfilter() const
{
	return Glare::Entity_manager<T...>::Component_range_const<U>{this};
}

template<typename... T>
typename Glare::Entity_manager<T...>::Range_const<>
	Glare::Entity_manager<T...>::cfilter() const
{
	return Glare::Entity_manager<T...>::Range_const<>{this};
}

template<typename... T>
template<typename... U, typename std::enable_if_t<(sizeof...(U) > 1), int>>
typename Glare::Entity_manager<T...>::Range_const<U...>
Glare::Entity_manager<T...>::filter() const
{
	return Glare::Entity_manager<T...>::Range_const<U...>{this};
}

template<typename... T>
template<typename U>
typename Glare::Entity_manager<T...>::Component_range_const<U>
Glare::Entity_manager<T...>::filter() const
{
	return Glare::Entity_manager<T...>::Component_range_const<U>{this};
}

template<typename... T>
typename Glare::Entity_manager<T...>::Range_const<>
Glare::Entity_manager<T...>::filter() const
{
	return Glare::Entity_manager<T...>::Range_const<>{this};
}

template<typename... T>
template<bool Is_const, typename... U>
template<bool Iter_const>
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Iterator_base<Iter_const>::Iterator_base
(std::conditional_t<Iter_const,
	const Entity_manager*, Entity_manager*> ptr,
	typename Slot_map<Entity>::template Iterator_base<Iter_const> iter,
	typename Slot_map<Entity>::template Iterator_base<Iter_const> end)
	:ptr {ptr},
	iter {iter},
	end {end}
{}

template<typename... T>
template<bool Is_const, typename... U>
template<bool Iter_const>
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Iterator_base<Iter_const>&
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Iterator_base<Iter_const>::operator++()
{
	using Stable_const_index =
		Glare::Slot_map<Glare::Entity_manager<T...>::Entity>::Stable_const_index;
	do {
		++iter;
	} while (iter != end && !ptr->has_component_impl<U...>(Stable_const_index {iter}));
	return *this;
}

template<typename ...T>
template<bool Is_const, typename... U>
template<bool Iter_const>
typename Glare::Entity_manager<T...>::Entity_iterator_base<Iter_const>
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Iterator_base<Iter_const>::operator*()
{
	return {ptr, iter};
}

template<typename... T>
template<bool Is_const, typename... U>
template<bool Iter_const>
template<bool V>
bool Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Iterator_base<Iter_const>::operator==(
	Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Iterator_base<V> rhs) const
{
	// probably don't need to compare pointers
	return iter == rhs.iter;
}

template<typename... T>
template<bool Is_const, typename... U>
template<bool Iter_const>
template<bool V>
bool Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Iterator_base<Iter_const>::operator!=(
	Glare::Entity_manager<T...>::Range_base<Is_const, U...>::Iterator_base<V> rhs) const
{
	// probably don't need to compare pointers
	return !(*this == rhs);
}

template<typename... T>
template<bool Is_const, typename... U>
std::conditional_t<Is_const,
	typename Glare::Entity_manager<T...>::Range_base<Is_const, U...>::const_iterator,
	typename Glare::Entity_manager<T...>::Range_base<Is_const, U...>::iterator>
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::begin() const
{
	return {ptr, ptr->ents.begin(), ptr->ents.end()};
}

template<typename... T>
template<bool Is_const, typename... U>
typename Glare::Entity_manager<T...>::Range_base<Is_const, U...>::const_iterator
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::cbegin() const
{
	return {ptr, ptr->ents.cbegin(), ptr->ents.cend()};
}

template<typename... T>
template<bool Is_const, typename... U>
std::conditional_t<Is_const,
	typename Glare::Entity_manager<T...>::Range_base<Is_const, U...>::const_iterator,
	typename Glare::Entity_manager<T...>::Range_base<Is_const, U...>::iterator>
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::end() const
{
	return {ptr, ptr->ents.end(), ptr->ents.end()};
}

template<typename... T>
template<bool Is_const, typename... U>
typename Glare::Entity_manager<T...>::Range_base<Is_const, U...>::const_iterator
Glare::Entity_manager<T...>::Range_base<Is_const, U...>::cend() const
{
	return {ptr, ptr->ents.cend(), ptr->ents.cend()};
}

template<typename... T>
template<bool Is_const, typename U>
template<bool Iter_const>
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Iterator_base<Iter_const>::Iterator_base
(std::conditional_t<Iter_const,
	const Glare::Entity_manager<T...>*, Glare::Entity_manager<T...>*> ptr,
typename Slot_map<Indexed_element<U>>::template Iterator_base<Iter_const> iter)
	:ptr {ptr},
	iter {iter}
{}

template<typename... T>
template<bool Is_const, typename U>
template<bool Iter_const>
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Iterator_base<Iter_const>&
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Iterator_base<Iter_const>::operator++()
{
	++iter;
	return *this;
}

template<typename ...T>
template<bool Is_const, typename U>
template<bool Iter_const>
typename Glare::Entity_manager<T...>::Component_iterator_base<Iter_const, U>
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Iterator_base<Iter_const>::operator*()
{
	return {ptr, iter};
}

template<typename... T>
template<bool Is_const, typename U>
template<bool Iter_const>
template<bool V>
bool Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Iterator_base<Iter_const>::operator==(
	Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Iterator_base<V> rhs) const
{
	return iter == rhs.iter;
}

template<typename... T>
template<bool Is_const, typename U>
template<bool Iter_const>
template<bool V>
bool Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Iterator_base<Iter_const>::operator!=(
	Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::Iterator_base<V> rhs) const
{
	return !(*this == rhs);
}

template<typename... T>
template<bool Is_const, typename U>
std::conditional_t<Is_const,
	typename Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::const_iterator,
	typename Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::iterator>
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::begin() const
{
	return {ptr, std::get<Glare::Slot_map<Indexed_element<U>>>(ptr->components).begin()};
}

template<typename... T>
template<bool Is_const, typename U>
typename Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::const_iterator
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::cbegin() const
{
	return {ptr, std::get<Glare::Slot_map<Indexed_element<U>>>(ptr->components).cbegin()};
}

template<typename... T>
template<bool Is_const, typename U>
std::conditional_t<Is_const,
	typename Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::const_iterator,
	typename Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::iterator>
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::end() const
{
	return {ptr, std::get<Glare::Slot_map<Indexed_element<U>>>(ptr->components).end()};
}

template<typename... T>
template<bool Is_const, typename U>
typename Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::const_iterator
Glare::Entity_manager<T...>::Component_range_base<Is_const, U>::cend() const
{
	return {ptr, std::get<Glare::Slot_map<Indexed_element<U>>>(ptr->components).cend()};
}

template<typename... T>
typename Glare::Entity_manager<T...>::Stable_index
Glare::Entity_manager<T...>::add()
{
	return ents.add();
}

template<typename... T>
template<typename... U>
bool Glare::Entity_manager<T...>::has_component
(typename Glare::Entity_manager<T...>::Stable_const_index e) const
{
	return has_component_impl<U...>(e);
}

template<typename... T>
template<typename... U>
bool Glare::Entity_manager<T...>::has_component
(typename Glare::Entity_manager<T...>::Stable_index e) const
{
	using Stable_const_index =
		Glare::Slot_map<Glare::Entity_manager<T...>::Entity>::Stable_const_index;
	return has_component_impl<U...>(Stable_const_index {e});
}

template<typename... T>
template<bool Is_const, typename V>
template<typename... U>
bool Glare::Entity_manager<T...>::Component_iterator_base<Is_const, V>::has_component() const
{
	return has_component_impl<U...>();
}

template<typename... T>
template<typename U>
const U& Glare::Entity_manager<T...>::component
(typename Glare::Entity_manager<T...>::Stable_const_index e) const
{
	return std::get<Glare::Slot_map<Indexed_element<U>>>(components)
		[std::get<Slot_map<Indexed_element<U>>::Stable_index>(ents[e].ptr)].val;
}

template<typename... T>
template<typename U>
U& Glare::Entity_manager<T...>::component
(typename Glare::Entity_manager<T...>::Stable_index e)
{
	return std::get<Glare::Slot_map<Indexed_element<U>>>(components)
		[std::get<Slot_map<Indexed_element<U>>::Stable_index>(ents[e].ptr)].val;
}

template<typename... T>
template<typename U>
U& Glare::Entity_manager<T...>::make_component
(typename Glare::Entity_manager<T...>::Stable_index e)
{
	using Elem_type = Glare::Slot_map<Indexed_element<U>>;
	auto& component_map = std::get<Elem_type>(components);
	auto& component_ptr = std::get<Elem_type::Stable_index>(ents[e].ptr);

	if (!component_map.is_valid(component_ptr)) {
		component_ptr = component_map.add(Indexed_element<U>{U {}, e});
	}
	return component_map[component_ptr].val;
}

template<typename... T>
template<typename U>
const U* Glare::Entity_manager<T...>::check_component
(typename Glare::Entity_manager<T...>::Stable_const_index e) const
{
	if (std::get<Glare::Slot_map<Indexed_element<U>>>(components)
		.is_valid(std::get<Glare::Slot_map<Indexed_element<U>>::Stable_index>(ents[e].ptr))) {
		return &(std::get<Glare::Slot_map<Indexed_element<U>>>(components)
				 [std::get<Glare::Slot_map<Indexed_element<U>>::Stable_index>(ents[e].ptr)].val);
	} else {
		return nullptr;
	}
}

template<typename... T>
template<typename U>
U* Glare::Entity_manager<T...>::check_component
(typename Glare::Entity_manager<T...>::Stable_index e)
{
	if (std::get<Glare::Slot_map<Indexed_element<U>>>(components)
		.is_valid(std::get<Glare::Slot_map<Indexed_element<U>>::Stable_index>(ents[e].ptr))) {
		return &(std::get<Glare::Slot_map<Indexed_element<U>>>(components)
				 [std::get<Glare::Slot_map<Indexed_element<U>>::Stable_index>(ents[e].ptr)].val);
	} else {
		return nullptr;
	}
}

template<typename... T>
template<typename First, typename... Rest>
bool Glare::Entity_manager<T...>::has_component_impl
(typename Glare::Entity_manager<T...>::Stable_const_index e) const
{
	return std::get<Glare::Slot_map<Indexed_element<First>>>(components).is_valid
	(std::get<Glare::Slot_map<Indexed_element<First>>::Stable_index>(ents[e].ptr))
		&& has_component_impl<Rest...>(e);
}

template<typename... T>
template<typename... Rest, typename>
bool Glare::Entity_manager<T...>::has_component_impl
(typename Glare::Entity_manager<T...>::Stable_const_index e) const
{
	return true;
}

template<typename... T>
template<bool Is_const, typename U>
template<typename First, typename... Rest,
	typename std::enable_if_t<std::is_same_v<First, U>, int>>
bool Glare::Entity_manager<T...>::Component_iterator_base<Is_const, U>::has_component_impl() const
{
	return has_component_impl<U, Rest...>();
}

template<typename... T>
template<bool Is_const, typename U>
template<typename First, typename... Rest,
	typename std::enable_if_t<!std::is_same_v<First, U>, int>>
	bool Glare::Entity_manager<T...>::Component_iterator_base<Is_const, U>::has_component_impl() const
{
	auto entity_index = ptr->ents[(*iter).index].ptr;

	return std::get<Glare::Slot_map<Indexed_element<First>>>(ptr->components).is_valid
		(std::get<Glare::Slot_map<Indexed_element<First>>::Stable_index>(entity_index))
		&& has_component_impl<Rest...>();
}

template<typename... T>
template<bool Is_const, typename U>
template<typename... Rest, typename>
bool Glare::Entity_manager<T...>::Component_iterator_base<Is_const, U>::has_component_impl() const
{
	return true;
}

#endif // !GLARE_ECS_HPP
