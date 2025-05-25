#include "gtest/gtest.h"

int main(int argc, char **argv) {
	static constexpr const char* reset_color = "\033[0m";
	static constexpr const char* bright_yellow_color = "\033[93m";
#ifdef NDEBUG
	std::cout << bright_yellow_color << "Running tests in Release configuration" << reset_color << '\n';
#else
	std::cout << bright_yellow_color << "Running tests in Debug configuration" << reset_color << '\n';
#endif

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
