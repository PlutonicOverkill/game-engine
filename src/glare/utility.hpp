#ifndef GLARE_UTILITY_HPP
#define GLARE_UTILITY_HPP

#include<tuple>
#include<vector>

namespace Glare{
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
	}
}

#endif // !GLARE_UTILITY_HPP
