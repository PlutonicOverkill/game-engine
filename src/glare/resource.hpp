#ifndef GLARE_RESOURCE_HPP
#define GLARE_RESOURCE_HPP

namespace Glare {
	/*
		Resources should inherit from this
	*/
	class Resource {

	};

	/*
		Use this to refer to a resource held by a Resource_manager
	*/
	class Resource_handle {
	public:

	private:
		Resource& res;
	};

	/*
		A manager class, how original
	*/
	class Resource_manager {

	};
}

#endif // !GLARE_RESOURCE_HPP
