#ifndef GLARE_GLARE_HPP
#define GLARE_GLARE_HPP

#include <tuple>
#include <type_traits>
#include <vector>

namespace Glare {
	namespace Video {

	}

	namespace Utility {
		template<typename... T>
		struct Typelist {};

		template<typename... T>
		struct List {};

		template<template<typename... Args> class U, typename... T>
		struct List<U<T...>> {
			using type = std::tuple<std::vector<T>...>;
		};

		template <typename T, typename... Ts>
		constexpr bool contains = (std::is_same<T, Ts>{} || ...);

		template <typename Subset, typename Set>
		constexpr bool is_subset_of = false;

		template <typename... Ts, typename... Us>
		constexpr bool is_subset_of<std::tuple<Ts...>, std::tuple<Us...>>
			= (contains<Ts, Us...> && ...);
	}

	namespace Ecs {
		template<typename... Components>
		class Entity {
		public:
			// returns nullptr if specified component doesn't exist
			template<typename T>
			T* component();
		private:
			unsigned version; // used to make sure Handle is up to date
		};

		template<typename... Components>
		class Ecs_handle {
		public:
			Entity* operator->();
			Entity operator*();
		private:
			Ecs* ptr;
			Ecs::size_type index;
			unsigned version;
		};

		template<typename... Components>
		class Ecs_iterator {
		public:
			// true if (ptr)
			operator bool();
		private:
			Entity* ptr;
		};

		template<typename... Components>
		class Ecs_const_iterator {
		public:
			// true if (ptr)
			operator bool();
		private:
			const Entity* ptr;
		};

		template<typename... Components>
		template<typename... Component_refs>
		class Ecs_ref_handle {
		public:

		private:
			Ecs_ref* ptr;
			Ecs_ref::size_type index;
			unsigned version;
		};

		template<typename... Components>
		template<typename... Component_refs>
		class Ecs_ref {
		public:
			using size_type = Ecs::size_type;

			friend class Ecs_ref_handle;

			class iterator {
			public:
				// true if (ptr) and pointer points to
				// an entity with relevant component types
				operator bool();
			private:
				Entity* ptr;
			};

			class const_iterator {
			public:
				// true if (ptr) and pointer points to
				// an entity with relevant component types
				operator bool();
			private:
				const Entity* ptr;
			};

			iterator begin();
			const_iterator begin() const;
			const_iterator cbegin() const;

			iterator end();
			const_iterator end() const;
			const_iterator cend() const;
		private:
			Ecs* ref;
		};

		/*
			System is a type with a () operator that is used as the type of systems
			Components is a list of all component types available

			Iterators, pointers, etc.:
				use Entity* to temporarily refer to an Entity
				use iterator to... well, iterate
				use Handle for a long-term handle to an entity
		*/
		template<typename Logic_sys, typename Render_sys, typename Components>
		class Ecs {};

		// specialization for tuples
		template<typename... Logic_sys, typename... Render_sys, typename... Components>
		class Ecs<std::tuple<Logic_sys...>,
			std::tuple<Render_sys...>,
			std::tuple<Components...>> {
		public:
			using size_type = size_t;
			using difference_type = ptrdiff_t;
			using value_type = Entity<Components...>;
			using iterator = Ecs_iterator<Components>;
			using const_iterator = Ecs_const_iterator<Components...>;
			using reference = Entity<Components...>&;
			using const_reference = const Entity<Components...>&;

			friend class Ecs_handle<Components...>;
			friend class Ecs_iterator<Components...>;
			friend class Ecs_const_iterator<Components...>;

			Handle add_entity();
			size_type size();

			template<typename... T>
			Ref<T...> sequence();

			Entity* operator[](size_type);

		private:
			Utility::List<Utility::Typelist<Logic_sys...>> logic_systems;
			Utility::List<Utility::Typelist<Render_sys...>> render_systems;

			Utility::List<Utility::Typelist<Components...>> components;
		};

	}


}

#endif // !GLARE_GLARE_HPP
