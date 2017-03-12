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

		struct Slot_index {
			template<typename T>
			Slot_map<T>::size_type index;
			int counter;
		};

		/*
			A container based on std::vector that does not invalidate
			Handles when elements are added/destroyed,
			although iterators, pointers and references are invalidated
		*/
		template<typename T>
		class Slot_map {
		public:
			Slot_map() = default;
			Slot_map(std::initializer_list<T>);
			Slot_map& operator=(std::initializer_list<T>);

			using value_type = std::vector<T>::value_type;
			using size_type = std::vector<T>::size_type;
			using difference_type = std::vector<T>::difference_type;
			using reference = std::vector<T>::reference;
			using const_reference = std::vector<T>::const_reference;
			using pointer = std::vector<T>::pointer;
			using const_pointer = std::vector<T>::const_pointer;
			using iterator = std::vector<T>::iterator;
			using const_iterator = std::vector<T>::const_iterator;
			using reverse_iterator = std::vector<T>::reverse_iterator;
			using const_reverse_iterator = std::vector<T>::const_reverse_iterator;

			iterator erase(const_iterator pos);
			iterator erase(const_iterator first, const_iterator last);

			void push_back(const T& value);
			void push_back(T&& value);

			template<class... Args>
			reference emplace_back(Args&&... args);
			void pop_back();

			void swap(Slot_map& other);

			reference operator[](size_type pos);
			const_reference operator[](size_type pos) const;

			reference at(size_type pos);
			const_reference at(size_type pos) const;

			reference back();
			const_reference back() const;

			reference front();
			const_reference front() const;

			iterator begin();
			const_iterator begin() const;
			const_iterator cbegin() const;

			iterator end();
			const_iterator end() const;
			const_iterator cend() const;

			reverse_iterator rbegin();
			const_reverse_iterator rbegin() const;
			const_reverse_iterator crbegin() const;

			reverse_iterator rend();
			const_reverse_iterator rend() const;
			const_reverse_iterator crend() const;

			bool empty() const;

			size_type size() const;

			void clear();

			// long-term handle, intended primarily for objects to safetly refer to others
			// "knows" which container it belongs to
			class Handle {
			public:
				Handle() = default;
				Handle(Slot_map*, int, int);
			private:
				Slot_map* ptr;
				// -1 in either field indicates "not valid"
				int index{-1};
				int counter{-1};
			};
		private:
			std::vector<T> elem;
			std::vector<Slot_index> elem_index;
			std::vector<size_type> free_index;

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
inline bool Glare::Utility::Slot_map<T>::empty() const
{
	return elem.empty();
}

template<typename T>
inline size_type Glare::Utility::Slot_map<T>::size() const
{
	return elem.size();
}

template<typename T>
Glare::Utility::Slot_map::Handle::Handle(Slot_map* ptr, int index, int counter)
	:ptr{ptr}, index{index}, counter{counter}
{}

#endif // !GLARE_GLARE_HPP
