#include "gtest/gtest.h"
#include "../glare/ecs.hpp"

struct Test1 {};
struct Test2 {};
struct Test3 {};

TEST(EntityManager, Constructor)
{
	Glare::Ecs::Entity_manager<Test1, Test2, Test3> em;
}
