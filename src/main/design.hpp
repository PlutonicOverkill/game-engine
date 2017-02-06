// if constexpr
// if(int x = 42; true != false)
// auto [a , b , c] = getvalues();
// template <auto v> struct S;

namespace fs = std::experimental::filesystem;

class Context {
	Context(); // init and shutdown opengl
	~Context();
	
	Context(const Context&) = delete;
	Context& operator=(const Context&) = delete;
	
	Context(Context&&) = default;
	Context& operator=(Context&&) = default;
}

class Renderer {
public:
	
private:
	Window win; // MUST be constructed in this order
	Context ctx;
	Asset_factory data;
};

template<typename T>
class Asset_factory {
	
};

class Asset {
public:
	virtual Asset() = 0;
	virtual ~Asset() = 0;
private:
	fs::path filename;
};

class gl_Asset : public Asset {
private:
	GLuint id;
};

class Texture : public gl_Asset {
public:
};

// mesh has to resend geometry data,
// so has to be stored in the object.
class Mesh : public gl_Asset {
public:
};

class Shader : public gl_Asset {
public:
};

// scene graph:
// base node with std::list of children
// geometry node
// transform node
// animated node: rotation, scaling, translating, animtransform
// switch node

/* when creating Shader:
 * first check if equivilant object exists
 * then check shader cache
 * then check file
 *
 * when deleting Shader:
 * remove from list of Shaders*/

// from http://gafferongames.com/game-physics/fix-your-timestep/ 
double t = 0.0; // time the simulation has been running (physics time)
constexpr double dt = 0.01; // constant physics update rate

double currentTime = hires_time_in_seconds(); // realtime
double accumulator = 0.0; // "leftover" time used for interpolating

State previous; // previous game state - used for interpolating
State current; // current game state

constexpr double maxtime = 0.25;

while (!quit) {
	double newTime = time();
	double frameTime = newTime - currentTime;
	if(frameTime > maxtime) // if sim gets too far behind, get it back up to date
		frameTime = maxtime;
	currentTime = newTime;

	accumulator += frameTime; // amount of time to simulate

	while (accumulator >= dt) {
		previousState = currentState;
		integrate(currentState, t, dt);
		t += dt;
		accumulator -= dt;
	}

	const double alpha = accumulator / dt;

	State state = currentState * alpha + 
		previousState * (1.0 - alpha);

	render(state);
}
