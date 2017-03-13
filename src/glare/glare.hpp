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

		template<typename T>
		struct Slot_index {
			typename Slot_map<T>::size_type index;
			int counter;
		};

		/*
			Slot_map
			Operations:
			Adding an element
				
			Removing an element
				
		*/
		template<typename T>
		class Slot_map {
		public:
			using value_type = T;
			using size_type = size_t;
			using difference_type = ptrdiff_t;

			// long-term handle, intended primarily for objects to safetly refer to others
			// "knows" which container it belongs to
			class Handle {
			public:
				Handle() = default;
				Handle(Slot_map*, int, int);
			private:
				Slot_map* ptr{nullptr};
				// -1 in either field indicates "not valid"
				int index{-1};
				int counter{-1};
			};

			// wrapper for std::pair<T, int>*
			template<bool Is_const>
			class iterator_base {
			public:
				iterator_base(std::pair<T, int>*);
				// default copy, move, destructor are fine

				// should these be const? they don't modify the iterator itself
				// but they can modify the object pointed to
				T* operator->() const;
				T& operator*() const;
				T& operator[](int) const;

				iterator_base& operator++();
				iterator_base& operator--();
				// no postfix operations for you

				iterator_base& operator+=(difference_type);
				iterator_base& operator-=(difference_type);

				iterator_base operator+(difference_type);
				iterator_base operator-(difference_type);

				template<bool U>
				difference_type operator-(iterator_base<U>);

				template<bool U>
				bool operator==(iterator_base<U>);
				template<bool U>
				bool operator!=(iterator_base<U>);

				explicit operator bool();
			private:
				std::conditional_t<Is_const,
					const std::pair<T, int>*, std::pair<T, int>*> ptr;
			};

			using iterator = iterator_base<false>;
			using const_iterator = iterator_base<true>;

			Slot_map() = default;
			Slot_map(std::initializer_list<T>);
			Slot_map& operator=(std::initializer_list<T>);

			// create
			// buffered_create
			// delete
			// buffered_delete
			// update (does all buffered_[creates|deletes])

		private:
			std::vector<std::pair<T, int>> elem; // .second is for index into elem_index
			std::vector<Slot_index<T>> elem_index;
			std::vector<size_type> free_index;

			// creation and deletion is buffered so that it does not invalidate iterators
			std::vector<Handle> deletion_buffer;
			std::vector<T> creation_buffer;

			// starts at 0 and increments each time an object is added
			// used to validate handles
			int counter{0};
		};
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

template<typename T>
bool Glare::Utility::Slot_map<T>::empty() const
{
	return elem.empty();
}

template<typename T>
Glare::Utility::Slot_map<T>::size_type Glare::Utility::Slot_map<T>::size() const
{
	return elem.size();
}

template<typename T>
Glare::Utility::Slot_map<T>::Handle::Handle(Slot_map<T>* ptr, int index, int counter)
	:ptr{ptr}, index{index}, counter{counter}
{}

#endif // !GLARE_GLARE_HPP
