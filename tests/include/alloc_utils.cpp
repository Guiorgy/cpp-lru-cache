#ifndef SKIP_ALLOCATION_TESTS
#include "alloc_utils.hpp"
#include <cstdlib>
#include <new>

std::atomic<std::size_t> allocation_count{0u};
std::atomic<std::size_t> allocated_bytes{0u};

void reset_allocation_count() noexcept {
	allocation_count.store(0u);
	allocated_bytes.store(0u);
}

void* operator new(std::size_t size) {
	++allocation_count;
	allocated_bytes += size;

	if (void* ptr = std::malloc(size)) {
		return ptr;
	} else {
		throw std::bad_alloc{};
	}
}

void operator delete(void* ptr) noexcept {
	std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
	std::free(ptr);
}

void* operator new[](std::size_t size) {
	++allocation_count;
	allocated_bytes += size;

	if (void* ptr = std::malloc(size)) {
		return ptr;
	} else {
		throw std::bad_alloc{};
	}
}

void operator delete[](void* ptr) noexcept {
	std::free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
	std::free(ptr);
}
#endif // SKIP_ALLOCATION_TESTS
