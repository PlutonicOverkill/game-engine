#ifndef GLARE_SYSTEM_HPP
#define GLARE_SYSTEM_HPP

#include "entity.hpp"

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace Glare {
/*
	System, updates relevant components
*/
	class System {
	public:
		// does the actual work of the system
		virtual void update(Entity::Entity ent, double dt) = 0;

		// returns the names of the components required
		// for an entity to be operated on by this system
		// Of course, there's nothing stopping a system from
		// modifying other components as well, but these are
		// the ones that are required
		virtual std::vector<std::string> components() = 0;

		virtual std::string name() = 0;

		// disabled to prevent slicing
		System(const System&) = delete;
		System& operator=(const System&) = delete;

		System(System&&) = delete;
		System& operator=(System&&) = delete;

		virtual ~System() {};
	};

	/*
		Manages all systems and updates them
		This is used so that systems can be split into groups
		For example, physics can be updated and rendering can be
		done seperately from each other
	*/
	class Subsystem {
	public:
		void update(double dt, Entity::Entity_container& ents)
		{
			for(const auto& sys : systems) {
				auto check = sys.second->components();
				for(auto& ent : ents) {
					bool update = true;
					for(const auto& x : check) {
						if(!ent.has_component(x)) {
							update = false;
							break;
						}
					}
					if(update) sys.second->update(ent, dt);
				}
			}

		}
	private:
		std::unordered_map<std::string, std::unique_ptr<System>> systems;
	};
}

#endif // !GLARE_SYSTEM_HPP
