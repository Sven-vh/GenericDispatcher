#include <svh/GenericDispatcher.hpp>
#include <string>
#include <iostream>

using json = std::string; // Placeholder for JSON type

template<typename T, typename...Args>
struct serialize_impl {
	//static json Run(const T&, Args&&...);
};

template<typename T, typename...Args>
struct visitable_serialize_impl {
	//static json Run(const T&, Args&&...);
};

template<typename T, typename...Args>
struct default_serialize_impl {
	//static json Run(const T&, Args&&...);
};


using JsonSerializerDispatcher =
svh::AnyDispatcher<
	serialize_impl,
	visitable_serialize_impl,
	default_serialize_impl
>;

struct Serializer {
	template<typename T, typename...Args>
	static json ToJson(const T& v, Args&&... args) {
		return JsonSerializerDispatcher::Run(
			v, std::forward<Args>(args)...);
	}
};

#define SERIALIZE_IMPL 1

#if SERIALIZE_IMPL == 1

template<>
struct serialize_impl<int, bool> {
	static json Run(const int& v, const bool test) {
		return test ? std::to_string(v) :
			"{\"int\": " + std::to_string(v) + "}";
	}
};

//#elif SERIALIZE_IMPL == 2

template<>
struct visitable_serialize_impl<int, bool> {
	static json Run(const int& v, const bool test) {
		return "{\"value\": " + std::to_string(v) + "}";
	}
};

#elif SERIALIZE_IMPL == 3

template<>
struct default_serialize_impl<int> {
	static json Run(const int& v) {
		return "{\"int\": " + std::to_string(v) + "}";
	}
};
#else

#error "Please define SERIALIZE_IMPL to choose a serialization implementation."

#endif

int main() {
	int value = 42;
	std::cout << "Serialized JSON: " << Serializer::ToJson(value, true) << std::endl;
	std::cout << "Serialized JSON: " << Serializer::ToJson(value, false) << std::endl;

	//wait for user input before exiting
	std::cout << "Press Enter to exit...";
	std::cin.get();

	return 0;
}