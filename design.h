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
	Context ctx; // MUST be constructed in this order
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

class gl_Asset : public Asset {};

class Texture : public gl_Asset {
public:
	
private:
	GLuint id;
};

class Mesh : public gl_Asset {
public:
	
private:
	GLuint id;
};

class Shader : public gl_Asset {
public:
	
private:
	GLuint id;
};

/* when creating Shader:
 * first check if equivilant object exists
 * then check shader cache
 * then check file
 *
 * when deleting Shader:
 * remove from list of Shaders*/

// from http://gafferongames.com/game-physics/fix-your-timestep/ 
double t = 0.0; // time the simulation has been running (physics time)
double dt = 0.01; // constant physics update rate

double currentTime = hires_time_in_seconds(); // realtime
double accumulator = 0.0; // "leftover" time used for interpolating

State previous; // previous game state - used for interpolating
State current; // current game state

while ( !quit )
{
    double newTime = time();
    double frameTime = newTime - currentTime;
    if ( frameTime > 0.25 ) // if sim gets too far behind, get it back up to date
        frameTime = 0.25;
    currentTime = newTime;

    accumulator += frameTime; // amount of time to simulate

    while ( accumulator >= dt )
    {
        previousState = currentState;
        integrate( currentState, t, dt );
        t += dt;
        accumulator -= dt;
    }

    const double alpha = accumulator / dt;

    State state = currentState * alpha + 
        previousState * ( 1.0 - alpha );

    render( state );
}
 