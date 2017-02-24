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
		bool has_component(std::string);
	private:
		// note: c++17 has try_emplace for non-copyable types
		std::unordered_map<std::string, std::unique_ptr<Component>> components;
	};

	/*
		Used to keep long-term safe handles to entities,
		usually stored in components of entities
		Don't keep them beyond the lifetime of the
		Entity_container, or stuff will break
	*/
	class Entity_handle {
	public:

	private:
		std::vector<Entity_index>::size_type slot_index; // index into slot_redirect
		unsigned int counter; // verify handle is up to date

		const Entity_container& container;
	};

/*
	Used to store entities
*/
	class Entity_container {
	public:
		friend class Entity_handle;

		Entity_handle add(Entity);
		void remove(Entity_handle);
	private:
		struct Entity_index {
			std::vector<Entity>::size_type ent_index; // index into ents
			unsigned int counter;
		};

		std::vector<Entity> ents;
		std::vector<Entity_index> slot_redirect;
		std::vector<int> free_slots;

		int counter;
	};
}

#endif // !GLARE_ENTITY_HPP
