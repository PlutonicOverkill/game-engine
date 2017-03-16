#ifndef GLARE_GLARE_HPP
#define GLARE_GLARE_HPP

/*
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#endif

#include "MyHeader.h"

#ifdef _MSC_VER
#pragma warning( default : 4244 )
#endif
*/

#include <tuple>
#include <type_traits>
#include <vector>
#include <exception>

namespace Glare {
	namespace Video {

	}

	namespace Error {
		class Glare_error : public std::runtime_error {};
	}

	namespace Utility {
		template<typename... T>
		struct Typelist {};

		template<typename... T>
		struct List {};

		// needed to get tuple of vectors with variadic template
		template<template<typename... Args> class U, typename... T>
		struct List<U<T...>> {
			using type = std::tuple<std::vector<T>...>;
		};

		template<typename T>
		class Slot_map;

		// long-term handle, intended primarily for objects to safetly refer to others
		// "knows" which container it belongs to
		template<typename T, bool Is_const>
		class Slot_map_pointer_base {
			using pointer_type = typename std::conditional_t<Is_const, const Slot_map*, Slot_map*>;
		public:
			Slot_map_pointer_base() = default; // doesn't point to a valid object
			Slot_map_pointer_base(pointer_type, int, int);
			// default copy, move, destructor are fine

			Slot_map_pointer_base& reset();
			explicit operator bool() const;

			const T* operator->() const;
			const T& operator*() const;

			T* operator->();
			T& operator*();

			template<bool U>
			bool operator==(Slot_map_pointer_base<U>);
			template<bool U>
			bool operator!=(Slot_map_pointer_base<U>);
		private:
			bool is_valid() const;

			pointer_type ptr;
			// -1 in either field indicates "not valid"
			Index index{-1};
			Counter counter{-1};
		};

		// wrapper for std::pair<T, int>*
		template<typename T, bool Is_const>
		class Slot_map_iterator_base {
			using iterator_type = typename std::conditional_t<Is_const, const Slot_map*, Slot_map*>;
		public:
			Slot_map_iterator_base(iterator_type, Direct_index);
			// default copy, move, destructor are fine

			const T* operator->() const;
			const T& operator*() const;
			const T& operator[](int) const;

			T* operator->();
			T& operator*();
			T& operator[](int);

			Slot_map_iterator_base& operator++();
			Slot_map_iterator_base& operator--();
			// no postfix operations for you

			Slot_map_iterator_base& operator+=(difference_type);
			Slot_map_iterator_base& operator-=(difference_type);

			template<bool U>
			difference_type operator-(Slot_map_iterator_base<U>);

			template<bool U>
			bool operator==(Slot_map_iterator_base<U>);
			template<bool U>
			bool operator!=(Slot_map_iterator_base<U>);
		private:
			iterator_type ptr;
			Direct_index index;
		};

		template<typename T>
		class Slot_map {
			using Index = int; // index to elem_indirect
			using Direct_index = int; // index to elem
			using Counter = int;
		public:
			using value_type = T;
			using size_type = size_t;
			using difference_type = ptrdiff_t;
			using pointer = Slot_map_pointer_base<T, false>;
			using const_pointer = Slot_map_pointer_base<T, true>;
			using iterator = Slot_map_iterator_base<T, false>;
			using const_iterator = Slot_map_iterator_base<T, true>;

			template<typename>
			friend class Slot_map_pointer_base<T>;
			template<typename>
			friend class Slot_map_iterator_base<T>;

			Slot_map() = default;
			Slot_map(std::initializer_list<T>);
			Slot_map& operator=(std::initializer_list<T>);

			// create
			// buffered_create
			// delete
			// buffered_delete
			// update (does all buffered_[creates|deletes])

		private:
			Slot_map& erase(Direct_index);

			size_type size();

			std::vector<std::pair<T, Index>> elem;
			std::vector<std::pair<Direct_index, Counter>> elem_indirect;
			std::vector<size_type> free_index;

			// creation and deletion is buffered so that it does not invalidate iterators
			std::vector<pointer> deletion_buffer;
			std::vector<std::pair<T, Index>> creation_buffer;

			// starts at 0 and increments each time an object is added
			// used to validate handles
			Counter counter{0};
		}; // Slot_map

		template<typename T>
		template<bool Is_const>
		Slot_map<T>::iterator_base<Is_const> operator+(Slot_map<T>::iterator_base<Is_const>, Slot_map<T>::difference_type);

		template<typename T>
		template<bool Is_const>
		Slot_map<T>::iterator_base<Is_const> operator-(Slot_map<T>::iterator_base<Is_const>, Slot_map<T>::difference_type);
}

	// Entity component system
	namespace Ecs {
		/*
			A manager class, how original
			T is a list of all the component types usable by Entities
		*/
		template<typename... T>
		class Entity_manager {

		};
	}
}

template<typename T, bool Is_const>
Glare::Utility::Slot_map_iterator_base<T, Is_const> Glare::Utility::operator+
(Glare::Utility::Slot_map_iterator_base<T, Is_const> lhs, Glare::Utility::Slot_map<T>::difference_type rhs)
{
	return lhs += rhs;
}

template<typename T, bool Is_const>
Glare::Utility::Slot_map_iterator_base<T, Is_const> Glare::Utility::operator-
(Glare::Utility::Slot_map_iterator_base<T, Is_const> lhs, Glare::Utility::Slot_map<T>::difference_type rhs)
{
	return lhs -= rhs;
}

template<typename T, bool Is_const>
inline Glare::Utility::Slot_map_iterator_base<T, Is_const>::Slot_map_iterator_base(iterator_type ptr, Direct_index index)
	:ptr{ptr},
	index{index}
{}

template<typename T, bool Is_const>
Glare::Utility::Slot_map_pointer_base<T, Is_const>::Slot_map_pointer_base(pointer_type ptr, int index, int counter)
	:ptr{ptr}, index{index}, counter{counter}
{}

template<typename T, bool Is_const>
const T* Glare::Utility::Slot_map_pointer_base<T, Is_const>::operator->() const
{
	auto redirect = ptr->elem_indirect[index];
	return &(ptr->elem[redirect].first());
}

template<typename T, bool Is_const>
T& Glare::Utility::Slot_map_pointer_base<T, Is_const>::operator*()
{
	auto redirect = ptr->elem_indirect[index];
	return ptr->elem[redirect].first();
}

template<typename T, bool Is_const>
T* Glare::Utility::Slot_map_pointer_base<T, Is_const>::operator->()
{
	static_assert(!Is_const, "pointer is constant");

	auto redirect = ptr->elem_indirect[index];
	return &(ptr->elem[redirect].first());
}

template<typename T, bool Is_const>
const T& Glare::Utility::Slot_map_pointer_base<T, Is_const>::operator*() const
{
	static_assert(!Is_const, "pointer is constant");

	auto redirect = ptr->elem_indirect[index];
	return ptr->elem[redirect].first();
}

template<typename T, bool Is_const>
bool Glare::Utility::Slot_map_pointer_base<T, Is_const>::is_valid() const
{
	// check that index and counter are in the correct range
	if (!ptr || counter == -1 || index < 0 || index >= ptr->elem_indirect.size())
		return false;

	auto redirect = ptr->elem_indirect[index];
	return redirect.second == counter; // check counters
}

template<typename T, bool Is_const>
inline Glare::Utility::Slot_map_pointer_base<T, Is_const>::operator bool() const
{
	return is_valid();
}

template<typename T, bool Is_const>
Slot_map_pointer_base & Glare::Utility::Slot_map_pointer_base<T, Is_const>::reset()
{
	ptr = nullptr;
	index = -1;
	counter = -1;
	return *this;
}

template<typename T, bool Is_const>
template<bool U>
inline bool Glare::Utility::Slot_map_pointer_base<T, Is_const>::operator==(Slot_map_pointer_base<U> rhs)
{
	return index == rhs.index
		&& counter == rhs.counter
		&& ptr == rhs.ptr;
}

template<typename T, bool Is_const>
template<bool U>
inline bool Glare::Utility::Slot_map_pointer_base<T, Is_const>::operator!=(Slot_map_pointer_base<U> rhs)
{
	return !(this == rhs);
}



template<typename T, bool Is_const>
inline const T* Glare::Utility::Slot_map_iterator_base<T, Is_const>::operator->() const
{
	return &(ptr->elem[index]);
}

template<typename T, bool Is_const>
inline const T& Glare::Utility::Slot_map_iterator_base<T, Is_const>::operator*() const
{
	return ptr->elem[index];
}

template<typename T, bool Is_const>
inline const T& Glare::Utility::Slot_map_iterator_base<T, Is_const>::operator[](int subscript) const
{
	return ptr->elem[index + subscript];
}

template<typename T, bool Is_const>
inline T* Glare::Utility::Slot_map_iterator_base<T, Is_const>::operator->()
{
	return &(ptr->elem[index]);
}

template<typename T, bool Is_const>
inline T& Glare::Utility::Slot_map_iterator_base<T, Is_const>::operator*()
{
	return ptr->elem[index];
}

template<typename T, bool Is_const>
inline T& Glare::Utility::Slot_map_iterator_base<T, Is_const>::operator[](int subscript)
{
	return ptr->elem[index + subscript];
}


/*
Main functions operate on Direct_indexes
Iterators call them directly
Pointer check their counters and then call them

Creation:
	push_back new elem
	SUB GETNEW
	if (!(freelist.empty())
		get elem_index from freelist
	else
		push_back new elem_index
	END SUB
	put index of elem
	set counter

Deletion:
	CHECK COUNTER
	elem_index of last elem = elem_index of removal elem
	elem_index of removal elem = -1
	swap elems
	pop

Buffered Creation:
	push_back new elem to add list
	CALL GETNEW
	set new elem's index to newly retrieved index
	set counter of elem_index to -1
	return pointer

Buffered Destruction:
	CHECK COUNTER
	push_back pointer to destroy list

Clean Creation:
	move elem from add_list to elem list
	get its index
	set that elem_index to point to actual element

Clean Deletion:
	CALL Deletion

Get
	Lookup but check counter
	(check index<size)
*/

#endif // !GLARE_GLARE_HPP
