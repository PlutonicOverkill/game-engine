#ifndef GLARE_ENTITY_HPP
#define GLARE_ENTITY_HPP

#include "component.hpp"
#include "entity.hpp"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace Glare {
		/*
			All game objects are of this type
		*/
	class Entity {
	public:
		void add_component(std::unique_ptr<Component>);
		Glare::Component* get_component(std::string);
	private:
		// note: c++17 has try_emplace for non-copyable types
		std::unordered_map<std::string, std::unique_ptr<Component>> components;
	};

	bool has_component(std::string);

	template<typename T>
	T& component(std::string);

	class Entity_container;

	/*
		Used to keep long-term safe handles to entities,
		usually stored in components of entities
		Don't keep them beyond the lifetime of the
		Entity_container, or stuff will break
	*/
	class Entity_handle {
	public:
		Entity_handle(size_t, unsigned, const Entity_container&);
	private:
		size_t slot_index; // index into slot_redirect
		unsigned counter; // verify handle is up to date

		const Entity_container* container;
	};

	/*
		Used to store entities
	*/
	class Entity_container {
	public:
		using value_type = ents.value_type;
		using reference = ents.reference;
		using const_reference = ents.const_reference;
		using pointer = ents.pointer;
		using const_pointer = ents.const_pointer;
		using size_type = ents.size_type;
		using difference_type = ents.difference_type;
		using iterator = ents.iterator;
		using const_iterator = ents.const_iterator;



		// long-term accessor functions, these can be stored
		Entity_handle get_handle(size_type);
		const Entity_handle get_handle(size_type);

		// standard accessor functions, short term only (eg. loops)
		Entity& at(size_type);
		Entity& operator[](size_type);

		const Entity& at(size_type) const;
		const Entity& operator[](size_type) const;

		iterator begin();
		const_iterator begin() const;

		iterator end();
		const_iterator end() const;

		// modification functions
		Entity_handle add(Entity);
		void remove(Entity_handle);

		// other functions
		size_type size() const;
	private:
		friend class Entity_handle;
		struct Entity_index {
			size_t ent_index; // index into ents
			unsigned int counter;
		};

		std::vector<Entity> ents;
		std::vector<Entity_index> slot_map;
		std::vector<int> free_slots;

		int counter{0};
	};
}

template<typename T>
T& Glare::component(std::string s)
{
	static_assert(std::is_base_of_v<Glare::Component, T>);
	return dynamic_cast<T&>(Glare::get_component(s));
}

#endif // !GLARE_ENTITY_HPP
