#include "gtest/gtest.h"

int main(int argc, char **argv) {
#ifdef NDEBUG
	std::cout << "Running tets in Release configuration\n";
#else
	std::cout << "Running tets in Debug configuration\n";
#endif

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
