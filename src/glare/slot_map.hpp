#ifndef GLARE_SLOT_MAP_HPP
#define GLARE_SLOT_MAP_HPP

#include "error.hpp"

#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>
#include <stdexcept>

namespace Glare {
	class Iterator_incorrect_range : public Error::Glare_error {};

	template<typename T>
	class Slot_map;

	template<typename T>
	class Slot_map {
		using Index = typename int; // index to elem_indirect
		using Direct_index = typename int; // index to elem
		using Counter = typename int;
	public:
		using value_type = typename T;
		using size_type = typename size_t;
		using difference_type = typename ptrdiff_t;

		// long-term handle, intended primarily for objects to safetly refer to others
		// "knows" which container it belongs to
		template<bool Is_const>
		class pointer_base {
			using pointer_type = typename std::conditional_t<Is_const, const Slot_map*, Slot_map*>;
		public:
			pointer_base() = default; // doesn't point to a valid object
			pointer_base(pointer_type, Index, Counter);
			// default copy, move, destructor are fine

			pointer_base& reset();
			explicit operator bool() const;

			const T* operator->() const;
			const T& operator*() const;

			T* operator->();
			T& operator*();

			void remove();
			void buffered_remove();

			template<bool U>
			bool operator==(pointer_base<U>);
			template<bool U>
			bool operator!=(pointer_base<U>);
		private:
			bool is_valid() const;

			pointer_type ptr;
			// -1 in either field indicates "not valid"
			Index index{-1};
			Counter counter{-1};
		}; // pointer_base

		   // wrapper for std::pair<T, int>*
		template<bool Is_const>
		class iterator_base {
			using iterator_type = typename std::conditional_t<Is_const, const Slot_map*, Slot_map*>;
		public:
			iterator_base(iterator_type, Direct_index);
			// default copy, move, destructor are fine

			const T* operator->() const;
			const T& operator*() const;
			const T& operator[](int) const;

			T* operator->();
			T& operator*();
			T& operator[](int);

			void remove();
			void buffered_remove();

			iterator_base& operator++();
			iterator_base& operator--();
			// no postfix operations for you

			iterator_base& operator+=(difference_type);
			iterator_base& operator-=(difference_type);

			template<bool U>
			difference_type operator-(iterator_base<U>);

			template<bool U>
			bool operator==(iterator_base<U>);
			template<bool U>
			bool operator!=(iterator_base<U>);
		private:
			iterator_type ptr;
			Direct_index index;
		}; // iterator_base

		using pointer = typename pointer_base<false>;
		using const_pointer = typename pointer_base<true>;
		using iterator = typename iterator_base<false>;
		using const_iterator = typename iterator_base<true>;

		Slot_map() = default;
		Slot_map(std::initializer_list<T>);
		Slot_map& operator=(std::initializer_list<T>);

		pointer add(T);
		Slot_map& remove(Direct_index);

		pointer buffered_add(T);
		Slot_map& buffered_remove(Direct_index);

		Slot_map& clean_buffers();

		void clear();
	private:
		void clean_add_buffer();
		void clean_remove_buffer();

		Index get_free();

		size_type size();

		std::vector<std::pair<T, Index>> elem;
		std::vector<std::pair<Direct_index, Counter>> elem_indirect;
		std::vector<Index> free_index;

		// creation and deletion is buffered so that it does not invalidate iterators
		std::vector<pointer> deletion_buffer;
		std::vector<std::pair<T, Index>> creation_buffer;

		// starts at 0 and increments each time an object is added
		// used to validate handles
		Counter counter{0};
	}; // Slot_map

	template<typename T, bool Is_const>
	Slot_map<T>::iterator_base<Is_const> operator+
		(Slot_map<T>::iterator_base<Is_const>, typename Slot_map<T>::difference_type);

	template<typename T, bool Is_const>
	Slot_map<T>::iterator_base<Is_const> operator-
		(Slot_map<T>::iterator_base<Is_const>, typename Slot_map<T>::difference_type);
}

/***** IMPLEMENTATION *****/

template<typename T, bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const> Glare::operator+
(Glare::Slot_map<T>::iterator_base<Is_const> lhs,
 typename Glare::Slot_map<T>::difference_type rhs)
{
	return lhs += rhs;
}

template<typename T, bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const> Glare::operator-
(Glare::Slot_map<T>::iterator_base<Is_const> lhs,
 typename Glare::Slot_map<T>::difference_type rhs)
{
	return lhs -= rhs;
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::iterator_base<Is_const>::iterator_base
(typename Glare::Slot_map<T>::iterator_base<Is_const>::iterator_type ptr,
 Direct_index index)
	:ptr{ptr},
	index{index}
{}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::pointer_base<Is_const>::pointer_base
(typename Glare::Slot_map<T>::pointer_base<Is_const>::pointer_type ptr,
 Index index, Counter counter)
	:ptr{ptr}, index{index}, counter{counter}
{}

template<typename T>
template<bool Is_const>
const T* Glare::Slot_map<T>::pointer_base<Is_const>::operator->() const
{
	auto redirect = ptr->elem_indirect[index];
	return &(ptr->elem[redirect].first());
}

template<typename T>
template<bool Is_const>
const T& Glare::Slot_map<T>::pointer_base<Is_const>::operator*() const
{
	auto redirect = ptr->elem_indirect[index];
	return ptr->elem[redirect].first();
}

template<typename T>
template<bool Is_const>
T& Glare::Slot_map<T>::pointer_base<Is_const>::operator*()
{
	static_assert(!Is_const, "pointer is constant");

	auto redirect = ptr->elem_indirect[index];
	return ptr->elem[redirect].first();
}

template<typename T>
template<bool Is_const>
T* Glare::Slot_map<T>::pointer_base<Is_const>::operator->()
{
	static_assert(!Is_const, "pointer is constant");

	auto redirect = ptr->elem_indirect[index];
	return &(ptr->elem[redirect].first());
}

template<typename T>
template<bool Is_const>
bool Glare::Slot_map<T>::pointer_base<Is_const>::is_valid() const
{
	// check that index and counter are in the correct range
	if (!ptr || counter == -1
		|| index < 0
		|| index >= ptr->elem_indirect.size())
		return false;

	auto redirect = ptr->elem_indirect[index];
	return redirect.second == counter // check counters
		&& redirect.first != -1; // check index is valid
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::pointer_base<Is_const>::operator bool() const
{
	return is_valid();
}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::pointer_base<Is_const>&
Glare::Slot_map<T>::pointer_base<Is_const>::reset()
{
	ptr = nullptr;
	index = -1;
	counter = -1;
	return *this;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::pointer_base<Is_const>::operator==
(Glare::Slot_map<T>::pointer_base<U> rhs)
{
	return index == rhs.index
		&& counter == rhs.counter
		&& ptr == rhs.ptr;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::pointer_base<Is_const>::operator!=
(Glare::Slot_map<T>::pointer_base<U> rhs)
{
	return !(this == rhs);
}

template<typename T>
template<bool Is_const>
const T* Glare::Slot_map<T>::iterator_base<Is_const>::operator->() const
{
	return &(ptr->elem[index]);
}

template<typename T>
template<bool Is_const>
const T& Glare::Slot_map<T>::iterator_base<Is_const>::operator*() const
{
	return ptr->elem[index];
}

template<typename T>
template<bool Is_const>
const T& Glare::Slot_map<T>::iterator_base<Is_const>::operator[](int subscript) const
{
	return ptr->elem[index + subscript];
}

template<typename T>
template<bool Is_const>
T* Glare::Slot_map<T>::iterator_base<Is_const>::operator->()
{
	return &(ptr->elem[index]);
}

template<typename T>
template<bool Is_const>
T& Glare::Slot_map<T>::iterator_base<Is_const>::operator*()
{
	return ptr->elem[index];
}

template<typename T>
template<bool Is_const>
T& Glare::Slot_map<T>::iterator_base<Is_const>::operator[](int subscript)
{
	return ptr->elem[index + subscript];
}

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
Glare::Slot_map<T>::iterator_base<Is_const>::operator-(iterator_base<U> rhs)
{
	if (ptr != rhs.ptr)
		throw Iterator_incorrect_range{};
	return index - rhs.index;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::iterator_base<Is_const>::operator==
(Glare::Slot_map<T>::iterator_base<U> rhs)
{
	return ptr == rhs.ptr && index == rhs.index;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::iterator_base<Is_const>::operator!=
(Glare::Slot_map<T>::iterator_base<U> rhs)
{
	return !(this == rhs);
}

template<typename T>
typename Glare::Slot_map<T>::size_type Glare::Slot_map<T>::size()
{
	return elem.size(); // should this be elem_indirect.size()?
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
	Index x = get_free();
	elem.emplace_back(t, x);
	elem_indirect[x] = {elem.size() - 1, counter};
	return {this, x, counter++};
}

template<typename T>
typename Glare::Slot_map<T>::Index Glare::Slot_map<T>::get_free()
{
	if (free_index.empty()) {
		elem_index.push_back({});
		return elem_index.size() - 1; // index of last element
	} else {
		Index x = free_index.back();
		free_index.pop_back();
		return x;
	}
}

template<typename T>
Glare::Slot_map<T>& Glare::Slot_map<T>::remove(Direct_index x)
{
	assert(!elem.empty());
	auto last_index = elem.back().second;
	auto remove_index = elem[x].second;

	// swap indices
	std::iter_swap(elem_indirect.begin() + remove_index, elem_indirect.begin() + last_index);
	elem_indirect[x].second = -1; // reset counter
	// swap element to be removed with last element and pop
	std::iter_swap(elem.begin() + x, elem.end());
	elem.pop_back();

	return *this;
}

template<typename T>
typename Glare::Slot_map<T>::pointer Glare::Slot_map<T>::buffered_add(T t)
{
	Index x = get_free();
	creation_buffer.emplace_back(t, x);
	elem_indirect[x] = {-1, counter};
	return {this, x, counter++};
}

template<typename T>
void Glare::Slot_map<T>::clean_add_buffer()
{
	while (!creation_buffer.empty()) {
		Index x = creation_buffer.back().second;
		elem.push_back(creation_buffer.back());
		creation_buffer.pop_back();
		elem_indirect[x].first = elem.size() - 1;
	}
}

template<typename T>
Glare::Slot_map<T>& Glare::Slot_map<T>::buffered_remove(Direct_index x)
{
	Index redirect = elem[x].second;
	deletion_buffer.emplace_back(this, redirect, elem_indirect[redirect].second);
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

template<typename T>
template<bool Is_const>
void Glare::Slot_map<T>::pointer_base<Is_const>::remove()
{
	if (is_valid) {
		Direct_index x = ptr->elem_indirect[index].first;
		ptr->remove(x);
	}
}

template<typename T>
template<bool Is_const>
void Glare::Slot_map<T>::pointer_base<Is_const>::buffered_remove()
{
	if (is_valid) {
		Direct_index x = ptr->elem_indirect[index].first;
		ptr->buffered_remove(x);
	}
}

template<typename T>
template<bool Is_const>
void Glare::Slot_map<T>::iterator_base<Is_const>::remove()
{
	ptr->remove(index);
}

template<typename T>
template<bool Is_const>
void Glare::Slot_map<T>::iterator_base<Is_const>::buffered_remove()
{
	ptr->buffered_remove(index);
}


#endif // !GLARE_SLOT_MAP_HPP
