#ifndef GLARE_SLOT_MAP_HPP
#define GLARE_SLOT_MAP_HPP

#include "error.hpp"

#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace Glare {
	template<typename T>
	class Slot_map {
		using Index = int; // index to elem_indirect
		using Direct_index = int; // index to elem
		using Counter = int;
	public:
		using value_type = T;
		using size_type = Direct_index;
		using difference_type = ptrdiff_t;

		using Out_of_range = Error::Slot_map_out_of_range;

		// long-term handle, intended primarily for objects to safetly refer to others
		// "knows" which container it belongs to
		template<bool Is_const>
		class pointer_base {
			using pointer_type = std::conditional_t<Is_const, const Slot_map*, Slot_map*>;
		public:
			using Not_valid = Error::Slot_map_pointer_not_valid;
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
			bool operator==(pointer_base<U>) const;
			template<bool U>
			bool operator!=(pointer_base<U>) const;

			template<bool U>
			explicit operator pointer_base<U>() const;

			bool is_valid() const;
		private:
			pointer_type ptr {nullptr};
			// -1 in either field indicates "not valid"
			Index index {-1};
			Counter counter {-1};
		}; // pointer_base

		template<bool Is_const>
		class iterator_base {
			using iterator_type = std::conditional_t<Is_const, const Slot_map*, Slot_map*>;
		public:
			iterator_base(iterator_type, Direct_index);
			// default copy, move, destructor are fine

			const T* operator->() const;
			const T& operator*() const;
			const T& operator[](int) const;

			T* operator->();
			T& operator*();
			T& operator[](int);

			void buffered_remove();

			iterator_base& operator++();
			iterator_base& operator--();
			// no postfix operations for you

			iterator_base& operator+=(difference_type);
			iterator_base& operator-=(difference_type);

			template<bool U>
			difference_type operator-(iterator_base<U>) const;

			template<bool U>
			bool operator==(iterator_base<U>) const;
			template<bool U>
			bool operator!=(iterator_base<U>) const;

			template<bool U>
			explicit operator iterator_base<U>() const;
			template<bool U>
			explicit operator pointer_base<U>();
		private:
			iterator_type ptr;
			Direct_index index;
		}; // iterator_base

		using pointer = pointer_base<false>;
		using const_pointer = pointer_base<true>;
		using iterator = iterator_base<false>;
		using const_iterator = iterator_base<true>;

		Slot_map() = default;
		Slot_map(std::initializer_list<T>);
		Slot_map& operator=(std::initializer_list<T>);

		pointer add(T);
		Slot_map& remove(Direct_index);

		pointer buffered_add(T);
		Slot_map& buffered_remove(Direct_index);

		Slot_map& clean_buffers();

		void clear();
		size_type size() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;

		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

		const T& operator[](Direct_index) const;
		T& operator[](Direct_index);
	private:
		void clean_add_buffer();
		void clean_remove_buffer();

		Index get_free();

		// will check if element with that index is scheduled for creation
		// returns -1 for "not found"
		int elem_in_creation_buffer(Index);

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
	:ptr {ptr},
	index {index}
{}

template<typename T>
template<bool Is_const>
Glare::Slot_map<T>::pointer_base<Is_const>::pointer_base
(typename Glare::Slot_map<T>::pointer_base<Is_const>::pointer_type ptr,
 Index index, Counter counter)
	:ptr {ptr},
	index {index},
	counter {counter}
{}

template<typename T>
template<bool Is_const>
const T& Glare::Slot_map<T>::pointer_base<Is_const>::operator*() const
{
	if (!is_valid()) throw Pointer_not_valid {"Invalid pointer dereferenced"};

	const Direct_index redirect {ptr->elem_indirect[index].index};
	return ptr->elem[redirect].val;
}

template<typename T>
template<bool Is_const>
T& Glare::Slot_map<T>::pointer_base<Is_const>::operator*()
{
	static_assert(!Is_const, "pointer is constant");
	if (!is_valid()) throw Not_valid {"Invalid pointer dereferenced"};

	const Direct_index redirect {ptr->elem_indirect[index].index};
	return ptr->elem[redirect].val;
}

template<typename T>
template<bool Is_const>
const T* Glare::Slot_map<T>::pointer_base<Is_const>::operator->() const
{
	if (!is_valid()) throw Pointer_not_valid {"Invalid pointer dereferenced"};

	const Direct_index redirect {ptr->elem_indirect[index].index};
	return &(ptr->elem[redirect].val);
}

template<typename T>
template<bool Is_const>
T* Glare::Slot_map<T>::pointer_base<Is_const>::operator->()
{
	static_assert(!Is_const, "pointer is constant");
	if (!is_valid()) throw Pointer_not_valid {"Invalid pointer dereferenced"};

	const Direct_index redirect {ptr->elem_indirect[index].index};
	return &(ptr->elem[redirect].val);
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

	const auto redirect = ptr->elem_indirect[index];
	return redirect.counter == counter // check counters
		&& redirect.index != -1; // check index is valid
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
(Glare::Slot_map<T>::pointer_base<U> rhs) const
{
	return index == rhs.index
		&& counter == rhs.counter
		&& ptr == rhs.ptr;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::pointer_base<Is_const>::operator!=
(Glare::Slot_map<T>::pointer_base<U> rhs) const
{
	return !(this == rhs);
}

template<typename T>
template<bool Is_const>
const T& Glare::Slot_map<T>::iterator_base<Is_const>::operator*() const
{
	return (*ptr)[index];
}

template<typename T>
template<bool Is_const>
T& Glare::Slot_map<T>::iterator_base<Is_const>::operator*()
{
	return (*ptr)[index];
}

template<typename T>
template<bool Is_const>
const T* Glare::Slot_map<T>::iterator_base<Is_const>::operator->() const
{
	return &((*ptr)[index]);
}

template<typename T>
template<bool Is_const>
T* Glare::Slot_map<T>::iterator_base<Is_const>::operator->()
{
	return &((*ptr)[index]);
}

template<typename T>
template<bool Is_const>
const T& Glare::Slot_map<T>::iterator_base<Is_const>::operator[](int subscript) const
{
	return (*ptr)[index + subscript];
}

template<typename T>
template<bool Is_const>
T& Glare::Slot_map<T>::iterator_base<Is_const>::operator[](int subscript)
{
	const auto temp = *this + subscript;
	if (!temp.is_valid()) throw Iterator_out_of_range {"Out of range access"};

	return (*ptr)[index + subscript];
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
Glare::Slot_map<T>::iterator_base<Is_const>::operator-(iterator_base<U> rhs) const
{
	if (ptr != rhs.ptr)
		throw Iterator_out_of_range {"Attempted to subtract iterators to different containers"};
	return index - rhs.index;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::iterator_base<Is_const>::operator==
(Glare::Slot_map<T>::iterator_base<U> rhs) const
{
	return ptr == rhs.ptr && index == rhs.index;
}

template<typename T>
template<bool Is_const>
template<bool U>
bool Glare::Slot_map<T>::iterator_base<Is_const>::operator!=
(Glare::Slot_map<T>::iterator_base<U> rhs) const
{
	return !(this == rhs);
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
typename Glare::Slot_map<T>::pointer Glare::Slot_map<T>::add(T t)
{
	const Index x {get_free()};
	elem.push_back({t, x});
	elem_indirect[x].index = elem.size() - 1;
	elem_indirect[x].counter = counter;
	return {this, x, counter++};
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
	elem_indirect[remove_index].counter = -1;
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
	elem_indirect[x] = {-1, counter};
	return {this, x, counter++};
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

template<typename T>
template<bool Is_const>
void Glare::Slot_map<T>::pointer_base<Is_const>::remove()
{
	if (is_valid()) {
		const Direct_index x {ptr->elem_indirect[index].index};
		ptr->remove(x);
		reset();
	}
}

template<typename T>
template<bool Is_const>
void Glare::Slot_map<T>::pointer_base<Is_const>::buffered_remove()
{
	if (is_valid()) {
		const Direct_index x {ptr->elem_indirect[index].index};
		ptr->buffered_remove(x);
		// reset();
	} else { // ugly due to lack of C++17 support
		const auto x = ptr->elem_in_creation_buffer(index);
		if (x != -1) {
			// element is being destroyed before it has been created

			// swap element to be removed with last element and pop
			std::swap(ptr->creation_buffer[x], ptr->creation_buffer.back());
			ptr->creation_buffer.pop_back();
		}
	}
}

template<typename T>
int Glare::Slot_map<T>::elem_in_creation_buffer(Index x)
{
	const auto iter = std::find_if(creation_buffer.begin(), creation_buffer.end(),
								   [x](Indexed_element p)
	{return p.index == x;});

	if (iter != creation_buffer.end()) {
		return iter - creation_buffer.begin();
	} else {
		return -1;
	}
}

template<typename T>
template<bool Is_const>
void Glare::Slot_map<T>::iterator_base<Is_const>::buffered_remove()
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
Glare::Slot_map<T>::iterator_base<Is_const>::operator Glare::Slot_map<T>::pointer_base<U>()
{
	const Index redirect {elem[index].index};
	const Counter count {elem_indirect[redirect].counter};
	return {ptr, redirect, counter};
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::pointer_base<Is_const>::operator
Glare::Slot_map<T>::pointer_base<U>() const
{
	return {ptr, index, counter};
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::iterator_base<Is_const>::operator
Glare::Slot_map<T>::iterator_base<U>() const
{
	return {ptr, index};
}

template<typename T>
const T& Glare::Slot_map<T>::operator[](Direct_index index) const
{
	if (index < 0 || index >= elem.size)
		throw Index_out_of_range("Slot_map indexed with out of range index");
	else
		return elem[index];
}

template<typename T>
T& Glare::Slot_map<T>::operator[](Direct_index index)
{
	if (index < 0 || index >= elem.size)
		throw Index_out_of_range("Slot_map indexed with out of range index");
	else
		return elem[index];
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::pointer_base<Is_const>::operator pointer_base<U>() const
{
	return {ptr, index, counter};
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::iterator_base<Is_const>::operator iterator_base<U>() const
{
	return {ptr, index};
}

template<typename T>
template<bool Is_const>
template<bool U>
Glare::Slot_map<T>::iterator_base<Is_const>::operator pointer_base<U>()
{
	auto redirect = ptr->elem_indirect[index];
	return {ptr, redirect.index, redirect.counter};
}

#endif // !GLARE_SLOT_MAP_HPP
