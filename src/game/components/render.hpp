#ifndef GAME_COMPONENTS_RENDER_HPP
#define GAME_COMPONENTS_RENDER_HPP

#include "../../glare/component.hpp"

#include <string>

namespace Game {
	namespace Component {
		struct Texture {};
		struct Mesh {};
		struct Shader {};

		struct Renderable : Glare::Component {
			Texture tex;
			Mesh mesh;
			Shader shader;
		};
	}
}

#endif // !GAME_COMPONENTS_RENDER_HPP
