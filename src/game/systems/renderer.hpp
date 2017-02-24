#ifndef GAME_SYSTEMS_RENDERER_HPP
#define GAME_SYSTEMS_RENDERER_HPP

namespace Glare::System {
	class Renderer : public System {
	};
}

class Renderer : public System {
public:
	void update(Entity ent) override
	{
		// do the stuff
	}
	
	std::vector<std::string> components() override
	{
		return {"Renderable", "Position"};
	}
	
	std::string name() override
	{
		return "Renderer";
	}
};

#endif // !GAME_SYSTEMS_RENDERER_HPP
