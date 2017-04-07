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
	class Slot_map;

	template<typename T, bool Const = false>
	class Stable_index_base {
	public:
		friend class Slot_map<T>;
		Stable_index_base() = default; // doesn't point to a valid object
		Stable_index_base(Slot_map::Direct_index, Slot_map::Counter);
		// default copy, move, destructor are fine

		Stable_index_base& reset();

		template<bool Rhs_const>
		bool operator==(Stable_index_base<T, Rhs_const>) const;
		template<bool Rhs_const>
		bool operator!=(Stable_index_base<T, Rhs_const>) const;

		template<bool Result_const>
		explicit operator Stable_index_base<T, Result_const>() const;
	private:
		Slot_map::Index index {null_index};
		Slot_map::Counter counter {null_index};
	}; // Stable_index

	template<typename T>
	class Slot_map {
		using Index = size_t; // index to elem_indirect
		using Direct_index = size_t; // index to elem
		using Counter = size_t;
		static constexpr Index null_index {std::numeric_limits<Index>::max()};
	public:
		using value_type = T;
		using size_type = Direct_index;
		using difference_type = int;
		using Not_valid = Error::Slot_map_index_not_valid;
		using Out_of_range = Error::Slot_map_out_of_range;

		using Stable_index = Stable_index_base<T>;
		using Const_stable_index = Stable_index<T, true>;

		Slot_map() = default;
		Slot_map(std::initializer_list<T>);
		Slot_map& operator=(std::initializer_list<T>);

		pointer add(T = {});
		// TODO: emplace
		Slot_map& remove(pointer);

		pointer buffered_add(T = {});
		// TODO: buffered_emplace
		Slot_map& buffered_remove(pointer);

		Slot_map& clean_buffers();

		void clear();
		size_type size() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;

		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

		Stable_index make_pointer(Direct_index) const;
		Const_stable_index make_const_pointer(Direct_index) const;

		template<bool Is_const>
		bool is_valid(pointer_base<Is_const>) const;

		const T& operator[](const_iterator) const;
		T& operator[](iterator);

		const T& operator[](const_pointer) const;
		T& operator[](pointer);
	private:
		void clean_add_buffer();
		void clean_remove_buffer();

		Index get_free();

		// will check if element with that index is scheduled for creation
		// returns null_index for "not found"
		Direct_index elem_in_creation_buffer(Index) const;

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
		std::vector<pointer> deletion_buffer;
		std::vector<Indexed_element> creation_buffer;

		// starts at 0 and increments each time an object is added
		// used to validate handles
		Counter counter {0};
	}; // Slot_map
}

/***** IMPLEMENTATION *****/

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>::operator+(difference_type rhs) const
{
	return {/*ptr, */index + rhs};
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>::operator-(difference_type rhs) const
{
	return {/*ptr*/, index - rhs};
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>::iterator_base
(/*typename Glare::Slot_map<T>::iterator_base<Is_const>::iterator_type ptr,*/
 Direct_index index)
	:/*ptr {ptr}*/,
	index {index}
{}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::pointer_base<Is_const>::pointer_base
(/*typename Glare::Slot_map<T>::pointer_base<Is_const>::pointer_type ptr,*/
 Index index, Counter counter)
	:/*ptr {ptr}*/,
	index {index},
	counter {counter}
{}

//template<typename T>
//template<bool Is_const>
//std::conditional_t<Is_const, const T&, T&> Glare::Slot_map<T>::pointer_base<Is_const>::operator*() const
//{
//	if (!is_valid()) throw Not_valid {"Invalid pointer dereferenced"};
//
//	const Direct_index redirect {ptr->elem_indirect[index].index};
//	return ptr->elem[redirect].val;
//}
//
//template<typename T>
//template<bool Is_const>
//std::conditional_t<Is_const, const T*, T*> Glare::Slot_map<T>::pointer_base<Is_const>::operator->() const
//{
//	if (!is_valid()) throw Not_valid {"Invalid pointer dereferenced"};
//
//	const Direct_index redirect {ptr->elem_indirect[index].index};
//	return &(ptr->elem[redirect].val);
//}

//template<typename T>
//template<bool Is_const>
//bool Glare::Slot_map<T>::pointer_base<Is_const>::is_valid() const
//{
//	// check that index and counter are in the correct range
//	if (!ptr || counter == Glare::Slot_map<T>::null_index
//		|| index < 0
//		|| index >= ptr->elem_indirect.size())
//		return false;
//
//	const auto redirect = ptr->elem_indirect[index];
//	return redirect.counter == counter // check counters
//		&& redirect.index != Glare::Slot_map<T>::null_index;
//}

//template<typename T>
//template<bool Is_const>
//Glare::Slot_map<T>::pointer_base<Is_const>::operator bool() const
//{
//	return is_valid();
//}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::pointer_base<Is_const>&
Glare::Slot_map<T>::pointer_base<Is_const>::reset()
{
	/*ptr = nullptr;*/
	index = Glare::Slot_map<T>::null_index;
	counter = Glare::Slot_map<T>::null_index;
	return *this;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::pointer_base<Is_const>::operator==
(Glare::Slot_map<T>::pointer_base<U> rhs) const
{
	return index == rhs.index
		&& counter == rhs.counter
		/*&& ptr == rhs.ptr*/;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::pointer_base<Is_const>::operator!=
(Glare::Slot_map<T>::pointer_base<U> rhs) const
{
	return !(*this == rhs);
}

//template<typename T>
//template<bool Is_const>
//template<bool U>
//bool Glare::Slot_map<T>::pointer_base<Is_const>::operator==
//(Glare::Slot_map<T>::iterator_base<U> rhs) const
//{
//	return *this == pointer_base<U>{rhs};
//}
//
//template<typename T>
//template<bool Is_const>
//template<bool U>
//bool Glare::Slot_map<T>::pointer_base<Is_const>::operator!=
//(Glare::Slot_map<T>::iterator_base<U> rhs) const
//{
//	return !(*this == rhs);
//}

//template<typename T>
//template<bool Is_const>
//std::conditional_t<Is_const, const T*, T*> Glare::Slot_map<T>::iterator_base<Is_const>::operator->() const
//{
//	return &((*ptr)[index]);
//}
//
//template<typename T>
//template<bool Is_const>
//std::conditional_t<Is_const, const T&, T&> Glare::Slot_map<T>::iterator_base<Is_const>::operator*() const
//{
//	return (*ptr)[index];
//}
//
//template<typename T>
//template<bool Is_const>
//std::conditional_t<Is_const, const T&, T&> Glare::Slot_map<T>::iterator_base<Is_const>::operator[](int subscript) const
//{
//	return (*ptr)[index + subscript];
//}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>&
Glare::Slot_map<T>::iterator_base<Is_const>::operator++()
{
	++index;
	return *this;
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>&
Glare::Slot_map<T>::iterator_base<Is_const>::operator--()
{
	--index;
	return *this;
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>&
Glare::Slot_map<T>::iterator_base<Is_const>::operator+=(difference_type rhs)
{
	index += rhs;
	return *this;
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>&
Glare::Slot_map<T>::iterator_base<Is_const>::operator-=(difference_type rhs)
{
	index -= rhs;
	return *this;
}

template<typename T>
template<bool Is_const>
template<bool U>
typename Glare::Slot_map<T>::difference_type
Glare::Slot_map<T>::iterator_base<Is_const>::operator-
(Glare::Slot_map<T>::iterator_base<U> rhs) const
{
	/*if (ptr != rhs.ptr)
		throw Out_of_range {"Attempted to subtract iterators to different containers"};*/
	return index - rhs.index;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::iterator_base<Is_const>::operator==
(Glare::Slot_map<T>::iterator_base<U> rhs) const
{
	return /*ptr == rhs.ptr && */index == rhs.index;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::iterator_base<Is_const>::operator!=
(Glare::Slot_map<T>::iterator_base<U> rhs) const
{
	return !(*this == rhs);
}

//template<typename T>
//template<bool Is_const>
//template<bool U>
//bool Glare::Slot_map<T>::iterator_base<Is_const>::operator==
//(Glare::Slot_map<T>::pointer_base<U> rhs) const
//{
//	return pointer_base<Is_const>{*this} == rhs;
//}
//
//template<typename T>
//template<bool Is_const>
//template<bool U>
//bool Glare::Slot_map<T>::iterator_base<Is_const>::operator!=
//(Glare::Slot_map<T>::pointer_base<U> rhs) const
//{
//	return !(*this == rhs);
//}

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
typename Glare::Slot_map<T>::pointer Glare::Slot_map<T>::add(T t)
{
	const Index x {get_free()};
	elem.push_back({t, x});
	elem_indirect[x].index = elem.size() - 1;
	elem_indirect[x].counter = counter;
	return {/*this, */x, counter++};
}

template<typename T>
typename Glare::Slot_map<T>::Index Glare::Slot_map<T>::get_free()
{
	if (free_index.empty()) {
		elem_indirect.push_back({});
		return elem_indirect.size() - 1; // index of last element
	} else {
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
typename Glare::Slot_map<T>::pointer Glare::Slot_map<T>::buffered_add(T t)
{
	const Index x {get_free()};
	creation_buffer.push_back({t, x});
	elem_indirect[x] = {Glare::Slot_map<T>::null_index, counter};
	return {/*this, */x, counter++};
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
	deletion_buffer.emplace_back(this, redirect, elem_indirect[redirect].counter);
	return *this;
}

template<typename T>
void Glare::Slot_map<T>::clean_remove_buffer()
{
	while (!deletion_buffer.empty()) {
		deletion_buffer.back().remove();
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

//template<typename T>
//template<bool Is_const>
//void Glare::Slot_map<T>::pointer_base<Is_const>::remove()
//{
//	if (is_valid()) {
//		const Direct_index x {ptr->elem_indirect[index].index};
//		ptr->remove(x);
//		reset();
//	}
//}

//template<typename T>
//template<bool Is_const>
//void Glare::Slot_map<T>::pointer_base<Is_const>::buffered_remove()
//{
//	if (is_valid()) {
//		const Direct_index x {ptr->elem_indirect[index].index};
//		ptr->buffered_remove(x);
//		// reset();
//	} else { // ugly due to lack of C++17 support
//		const auto x = ptr->elem_in_creation_buffer(index);
//		if (x != Glare::Slot_map<T>::null_index) {
//			// element is being destroyed before it has been created
//
//			// swap element to be removed with last element and pop
//			std::swap(ptr->creation_buffer[x], ptr->creation_buffer.back());
//			ptr->creation_buffer.pop_back();
//		}
//	}
//}

template<typename T>
typename Glare::Slot_map<T>::Direct_index
Glare::Slot_map<T>::elem_in_creation_buffer(Index x)
{
	const auto iter = std::find_if(creation_buffer.begin(), creation_buffer.end(),
								   [x](Indexed_element p)
	{return p.index == x;});

	if (iter != creation_buffer.end()) {
		return iter - creation_buffer.begin();
	} else {
		return Glare::Slot_map<T>::null_index;
	}
}

//template<typename T>
//template<bool Is_const>
//void Glare::Slot_map<T>::iterator_base<Is_const>::buffered_remove()
//{
//	ptr->buffered_remove(index);
//}

template<typename T>
typename Glare::Slot_map<T>::iterator Glare::Slot_map<T>::begin()
{
	return {/*this, */0};
}

template<typename T>
typename Glare::Slot_map<T>::const_iterator Glare::Slot_map<T>::begin() const
{
	return {/*this, */0};
}

template<typename T>
typename Glare::Slot_map<T>::const_iterator Glare::Slot_map<T>::cbegin() const
{
	return {/*this, */0};
}

template<typename T>
typename Glare::Slot_map<T>::iterator Glare::Slot_map<T>::end()
{
	return {/*this, */size()};
}

template<typename T>
typename Glare::Slot_map<T>::const_iterator Glare::Slot_map<T>::end() const
{
	return {/*this, */size()};
}

template<typename T>
typename Glare::Slot_map<T>::const_iterator Glare::Slot_map<T>::cend() const
{
	return {/*this, */size()};
}

//template<typename T>
//template<bool Is_const>
//template<bool U>
//Glare::Slot_map<T>::iterator_base<Is_const>::operator
//Glare::Slot_map<T>::pointer_base<U>() const
//{
//	const Checked_index redirect {ptr->elem_indirect[index]};
//	return {/*ptr, */redirect.index, redirect.counter};
//}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::pointer_base<Is_const>::operator
Glare::Slot_map<T>::pointer_base<U>() const
{
	return {/*ptr, */index, counter};
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::iterator_base<Is_const>::operator
Glare::Slot_map<T>::iterator_base<U>() const
{
	return {/*ptr, */index};
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
