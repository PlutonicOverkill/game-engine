#ifndef GLARE_SLOT_MAP_HPP
#define GLARE_SLOT_MAP_HPP

#include "error.hpp"

#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <limits>

namespace Glare {
	template<typename T>
	class Slot_map {
		using Index = size_t; // index to elem_indirect
		using Direct_index = size_t; // index to elem
		using Counter = size_t;
		static constexpr size_t null_index {std::numeric_limits<size_t>::max()};
	public:
		using value_type = T;
		using size_type = Direct_index;
		using difference_type = int;

		using Not_valid = Error::Slot_map_stable_index_not_valid;
		using Out_of_range = Error::Slot_map_out_of_range;

		template<bool Is_const>
		class Iterator_base;

		// long-term handle
		// intended primarily for objects to safely refer to others
		template<bool Is_const>
		class Index_base {
		public:
			// allow members to access both versions of Index_base
			template<bool U>
			friend class Index_base;
			// HACK: messy but works
			// for equality check between indexes and iterators
			template<bool U>
			friend class Slot_map<T>::Iterator_base;

			friend class Slot_map<T>;

			Index_base() = default; // doesn't point to a valid object
			Index_base(Direct_index, Counter);
			// default copy, move, destructor are fine

			Index_base& reset();

			template<bool U>
			bool operator==(Index_base<U>) const;
			template<bool U>
			bool operator!=(Index_base<U>) const;

			template<bool U>
			bool operator==(Slot_map<T>::Iterator_base<U>) const;
			template<bool U>
			bool operator!=(Slot_map<T>::Iterator_base<U>) const;

			template<bool U>
			explicit operator Index_base<U>() const;
		private:
			Index index {null_index};
			Counter counter {null_index};
			// not storing a pointer complicates the implementation
			// but it just seems wrong having hundreds of pointers
			// lying around all pointing to the same object 
		}; // Index_base

		template<bool Is_const>
		class Iterator_base {
			using iterator_type = std::conditional_t<Is_const, const Slot_map*, Slot_map*>;
		public:
			// allow members to access both versions of Iterator_base
			template<bool U>
			friend class Iterator_base;
			// HACK: messy but works
			// for equality check between indexes and iterators
			template<bool U>
			friend class Slot_map<T>::Index_base;

			Iterator_base(iterator_type, Direct_index);
			// default copy, move, destructor are fine

			std::conditional_t<Is_const, const T*, T*> operator->() const;
			std::conditional_t<Is_const, const T&, T&> operator*() const;
			std::conditional_t<Is_const, const T&, T&> operator[](int) const;

			void buffered_remove();

			Iterator_base& operator++();
			Iterator_base& operator--();
			// no postfix operations for you

			Iterator_base& operator+=(difference_type);
			Iterator_base& operator-=(difference_type);

			// TODO: these shouldn't be members, but apparently
			// global operators can't have multiple template
			// parameter lists, so they have to be here
			Iterator_base operator+(difference_type) const;
			Iterator_base operator-(difference_type) const;

			template<bool U>
			difference_type operator-(Iterator_base<U>) const;

			template<bool U>
			bool operator==(Iterator_base<U>) const;
			template<bool U>
			bool operator!=(Iterator_base<U>) const;

			template<bool U>
			bool operator==(Slot_map<T>::Index_base<U>) const;
			template<bool U>
			bool operator!=(Slot_map<T>::Index_base<U>) const;

			template<bool U>
			explicit operator Iterator_base<U>() const;
			template<bool U>
			explicit operator Index_base<U>() const;
		private:
			iterator_type ptr;
			Direct_index index;
		}; // Iterator_base

		using Stable_index = Index_base<false>;
		using Stable_const_index = Index_base<true>;
		using iterator = Iterator_base<false>;
		using const_iterator = Iterator_base<true>;

		Slot_map() = default;
		Slot_map(std::initializer_list<T>);
		Slot_map& operator=(std::initializer_list<T>);

		Stable_index add(T = {});
		// TODO: emplace
		Slot_map& remove(Direct_index);
		Slot_map& remove(Stable_index);

		Stable_index buffered_add(T = {});
		// TODO: buffered_emplace
		Slot_map& buffered_remove(Direct_index);
		Slot_map& buffered_remove(Stable_index);

		Slot_map& clean_buffers();

		void clear();
		size_type size() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;

		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

		const T& operator[](Stable_const_index) const;
		T& operator[](Stable_index);

		template<bool Is_const>
		bool is_valid(Index_base<Is_const>) const;

		const T& operator[](Direct_index) const;
		T& operator[](Direct_index);
	private:
		void clean_add_buffer();
		void clean_remove_buffer();

		Index get_free();

		// will check if element with that index is scheduled for creation
		// returns null_index for "not found"
		Direct_index elem_in_creation_buffer(Index);

		struct Indexed_element {
			T val;
			Index index;
		};

		struct Checked_index {
			Direct_index index;
			Counter counter;
		};

		std::vector<Indexed_element> elem;
		std::vector<Checked_index> elem_indirect;
		std::vector<Index> free_index;

		// creation and deletion is buffered so that it does not invalidate iterators
		std::vector<Stable_index> deletion_buffer;
		std::vector<Indexed_element> creation_buffer;

		// starts at 0 and increments each time an object is added
		// used to validate handles
		Counter counter {0};
	}; // Slot_map
}

/***** IMPLEMENTATION *****/

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>::operator+(difference_type rhs) const
{
	return {ptr, index + rhs};
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>::operator-(difference_type rhs) const
{
	return {ptr, index - rhs};
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>::Iterator_base
(typename Glare::Slot_map<T>::Iterator_base<Is_const>::iterator_type ptr,
 Direct_index index)
	:ptr {ptr},
	index {index}
{}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Index_base<Is_const>::Index_base
(Index index, Counter counter)
	:index {index},
	counter {counter}
{}

template<typename T>
const T& Glare::Slot_map<T>::operator[]
(typename Glare::Slot_map<T>::Stable_const_index p) const
{
	if (!is_valid(p)) throw Not_valid {"Invalid Stable_index dereferenced"};

	const Direct_index redirect {elem_indirect[p.index].index};
	return elem[redirect].val;
}

template<typename T>
T& Glare::Slot_map<T>::operator[]
(typename Glare::Slot_map<T>::Stable_index p)
{
	if (!is_valid(p)) throw Not_valid {"Invalid Stable_index dereferenced"};

	const Direct_index redirect {elem_indirect[p.index].index};
	return elem[redirect].val;
}

template<typename T>
template<bool Is_const>
bool Glare::Slot_map<T>::is_valid(Glare::Slot_map<T>::Index_base<Is_const> p) const
{
	// check that index and counter are in the correct range
	if (p.counter == Glare::Slot_map<T>::null_index
		|| p.index < 0
		|| p.index >= elem_indirect.size())
		return false;

	const auto redirect = elem_indirect[p.index];
	return redirect.counter == p.counter // check counters
		&& redirect.index != Glare::Slot_map<T>::null_index;
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Index_base<Is_const>&
Glare::Slot_map<T>::Index_base<Is_const>::reset()
{
	index = Glare::Slot_map<T>::null_index;
	counter = Glare::Slot_map<T>::null_index;
	return *this;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::Index_base<Is_const>::operator==
(Glare::Slot_map<T>::Index_base<U> rhs) const
{
	return index == rhs.index
		&& counter == rhs.counter;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::Index_base<Is_const>::operator!=
(Glare::Slot_map<T>::Index_base<U> rhs) const
{
	return !(*this == rhs);
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::Index_base<Is_const>::operator==
(Glare::Slot_map<T>::Iterator_base<U> rhs) const
{
	return *this == Index_base<U>{rhs};
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::Index_base<Is_const>::operator!=
(Glare::Slot_map<T>::Iterator_base<U> rhs) const
{
	return !(*this == rhs);
}

template<typename T>
template<bool Is_const>
std::conditional_t<Is_const, const T*, T*> Glare::Slot_map<T>::Iterator_base<Is_const>::operator->() const
{
	return &((*ptr)[index]);
}

template<typename T>
template<bool Is_const>
std::conditional_t<Is_const, const T&, T&> Glare::Slot_map<T>::Iterator_base<Is_const>::operator*() const
{
	return (*ptr)[index];
}

template<typename T>
template<bool Is_const>
std::conditional_t<Is_const, const T&, T&> Glare::Slot_map<T>::Iterator_base<Is_const>::operator[](int subscript) const
{
	return (*ptr)[index + subscript];
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>&
Glare::Slot_map<T>::Iterator_base<Is_const>::operator++()
{
	++index;
	return *this;
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>&
Glare::Slot_map<T>::Iterator_base<Is_const>::operator--()
{
	--index;
	return *this;
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>&
Glare::Slot_map<T>::Iterator_base<Is_const>::operator+=(difference_type rhs)
{
	index += rhs;
	return *this;
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::Iterator_base<Is_const>&
Glare::Slot_map<T>::Iterator_base<Is_const>::operator-=(difference_type rhs)
{
	index -= rhs;
	return *this;
}

template<typename T>
template<bool Is_const>
template<bool U>
typename Glare::Slot_map<T>::difference_type
Glare::Slot_map<T>::Iterator_base<Is_const>::operator-
(Glare::Slot_map<T>::Iterator_base<U> rhs) const
{
	if (ptr != rhs.ptr)
		throw Out_of_range {"Attempted to subtract iterators to different containers"};
	return index - rhs.index;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::Iterator_base<Is_const>::operator==
(Glare::Slot_map<T>::Iterator_base<U> rhs) const
{
	return ptr == rhs.ptr && index == rhs.index;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::Iterator_base<Is_const>::operator!=
(Glare::Slot_map<T>::Iterator_base<U> rhs) const
{
	return !(*this == rhs);
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::Iterator_base<Is_const>::operator==
(Glare::Slot_map<T>::Index_base<U> rhs) const
{
	return Index_base<Is_const>{*this} == rhs;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::Iterator_base<Is_const>::operator!=
(Glare::Slot_map<T>::Index_base<U> rhs) const
{
	return !(*this == rhs);
}

template<typename T>
typename Glare::Slot_map<T>::size_type Glare::Slot_map<T>::size() const
{
	return elem.size();
}

template<typename T>
Glare::Slot_map<T>& Glare::Slot_map<T>::clean_buffers()
{
	// remove first so that memory is not pointlessly allocated
	clean_remove_buffer();
	clean_add_buffer();
	return *this;
}

template<typename T>
Glare::Slot_map<T>::Slot_map(std::initializer_list<T> init)
{
	for (auto x : init) {
		add(x);
	}
}

template<typename T>
Glare::Slot_map<T>& Glare::Slot_map<T>::operator=(std::initializer_list<T> init)
{
	clear();
	for (auto x : init) {
		add(x);
	}
}

template<typename T>
typename Glare::Slot_map<T>::Stable_index Glare::Slot_map<T>::add(T t)
{
	const Index x {get_free()};
	elem.push_back({t, x});
	elem_indirect[x].index = elem.size() - 1;
	elem_indirect[x].counter = counter;
	return {x, counter++};
}

template<typename T>
typename Glare::Slot_map<T>::Index Glare::Slot_map<T>::get_free()
{
	if (free_index.empty()) {
		elem_indirect.push_back({});
		return elem_indirect.size() - 1; // index of last element
	}
	else {
		const Index x {free_index.back()};
		free_index.pop_back();
		return x;
	}
}

template<typename T>
Glare::Slot_map<T>& Glare::Slot_map<T>::remove(Direct_index x)
{
	assert(!elem.empty());
	const Index last_index = elem.back().index;
	const Index remove_index = elem[x].index;

	// reset counter
	elem_indirect[remove_index].counter = Glare::Slot_map<T>::null_index;
	// swap indices
	std::swap(elem_indirect[remove_index].index, elem_indirect[last_index].index);

	// swap element to be removed with last element and pop
	std::swap(elem[x], elem.back());
	elem.pop_back();

	return *this;
}

template<typename T>
typename Glare::Slot_map<T>::Stable_index Glare::Slot_map<T>::buffered_add(T t)
{
	const Index x {get_free()};
	creation_buffer.push_back({t, x});
	elem_indirect[x] = {Glare::Slot_map<T>::null_index, counter};
	return {x, counter++};
}

template<typename T>
void Glare::Slot_map<T>::clean_add_buffer()
{
	while (!creation_buffer.empty()) {
		const Index x {creation_buffer.back().index};
		elem.push_back(creation_buffer.back());
		creation_buffer.pop_back();
		elem_indirect[x].index = elem.size() - 1;
	}
}

template<typename T>
Glare::Slot_map<T>& Glare::Slot_map<T>::buffered_remove(Direct_index x)
{
	const Index redirect {elem[x].index};
	deletion_buffer.emplace_back(redirect, elem_indirect[redirect].counter);
	return *this;
}

template<typename T>
void Glare::Slot_map<T>::clean_remove_buffer()
{
	while (!deletion_buffer.empty()) {
		remove(deletion_buffer.back());
		deletion_buffer.pop_back();
	}
}

template<typename T>
void Glare::Slot_map<T>::clear()
{
	elem.clear();
	elem_indirect.clear();
	free_index.clear();
	deletion_buffer.clear();
	creation_buffer.clear();
}

template<typename T>
Glare::Slot_map<T>& Glare::Slot_map<T>::remove
(typename Glare::Slot_map<T>::Stable_index p)
{
	if (is_valid(p)) {
		const Direct_index x {elem_indirect[p.index].index};
		remove(x);
		p.reset();
	}
	return *this;
}

template<typename T>
Glare::Slot_map<T>& Glare::Slot_map<T>::buffered_remove
(typename Glare::Slot_map<T>::Stable_index p)
{
	if (is_valid(p)) {
		const Direct_index x {elem_indirect[p.index].index};
		buffered_remove(x);
		p.reset();
	} else { // ugly due to lack of C++17 support
		const auto x = elem_in_creation_buffer(p.index);
		if (x != Glare::Slot_map<T>::null_index) {
			// element is being destroyed before it has been created

			// swap element to be removed with last element and pop
			std::swap(creation_buffer[x], creation_buffer.back());
			creation_buffer.pop_back();
		}
	}
	return *this;
}

template<typename T>
typename Glare::Slot_map<T>::Direct_index
Glare::Slot_map<T>::elem_in_creation_buffer(Index x)
{
	const auto iter = std::find_if(creation_buffer.begin(), creation_buffer.end(),
								   [x](Indexed_element p) {return p.index == x; });

	if (iter != creation_buffer.end()) {
		return iter - creation_buffer.begin();
	}
	else {
		return Glare::Slot_map<T>::null_index;
	}
}

template<typename T>
template<bool Is_const>
void Glare::Slot_map<T>::Iterator_base<Is_const>::buffered_remove()
{
	ptr->buffered_remove(index);
}

template<typename T>
typename Glare::Slot_map<T>::iterator Glare::Slot_map<T>::begin()
{
	return {this, 0};
}

template<typename T>
typename Glare::Slot_map<T>::const_iterator Glare::Slot_map<T>::begin() const
{
	return {this, 0};
}

template<typename T>
typename Glare::Slot_map<T>::const_iterator Glare::Slot_map<T>::cbegin() const
{
	return {this, 0};
}

template<typename T>
typename Glare::Slot_map<T>::iterator Glare::Slot_map<T>::end()
{
	return {this, size()};
}

template<typename T>
typename Glare::Slot_map<T>::const_iterator Glare::Slot_map<T>::end() const
{
	return {this, size()};
}

template<typename T>
typename Glare::Slot_map<T>::const_iterator Glare::Slot_map<T>::cend() const
{
	return {this, size()};
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::Iterator_base<Is_const>::operator
Glare::Slot_map<T>::Index_base<U>() const
{
	const Checked_index redirect {ptr->elem_indirect[index]};
	return {redirect.index, redirect.counter};
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::Index_base<Is_const>::operator
Glare::Slot_map<T>::Index_base<U>() const
{
	return {index, counter};
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::Iterator_base<Is_const>::operator
Glare::Slot_map<T>::Iterator_base<U>() const
{
	return {ptr, index};
}

template<typename T>
const T& Glare::Slot_map<T>::operator[](Direct_index index) const
{
	if (index < 0 || index >= elem.size())
		throw Out_of_range("Slot_map indexed with out of range index");
	else
		return elem[index].val;
}

template<typename T>
T& Glare::Slot_map<T>::operator[](Direct_index index)
{
	if (index < 0 || index >= elem.size())
		throw Out_of_range("Slot_map indexed with out of range index");
	else
		return elem[index].val;
}

#endif // !GLARE_SLOT_MAP_HPP
