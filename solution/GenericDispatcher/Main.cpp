#include <svh/GenericDispatcher.hpp>
#include <string>
#include <iostream>

/* My library */
namespace MyLogger {
	template<typename T, typename...Args>
	struct user_impl {};

	template<typename T, typename...Args>
	struct library_impl {};

	template<typename T, typename...Args>
	struct fallback_impl {
		static std::string Run(const T& value, Args&&... args) {
			return "{Uknow Type}";
			/* Or */
			//static_assert(svh::always_false<T>::value, "No matching provider for this type/args!");
		}
	};

	using LoggerDispatcher = svh::AnyDispatcher<
		user_impl, /* First check if the user has implemented it */
		library_impl, /* Then check if the library has an implementation */
		fallback_impl /* Else, use a fallback implementation */
	>;

	template<typename T, typename... Args>
	static std::string Log(const T& value, Args&&... args) {
		return LoggerDispatcher::Run(value, std::forward<Args>(args)...);
	}


	/* Library implementations */
	template<>
	struct library_impl<int> {
		static std::string Run(const int& value) {
			return std::to_string(value);
		}
	};

	template<>
	struct library_impl<bool> {
		static std::string Run(const bool& value) {
			return value ? "true" : "false";
		}
	};
}

/* User decides they don't like the way bools are logged */
/* They can just "override" it */
namespace MyLogger {
	template<>
	struct user_impl<bool> {
		static std::string Run(const bool& value) {
			return value ? "yes" : "no";
		}
	};
}

struct MyStruct {
	int a;
	bool b;
	float c;
};

/* User can also implement their own types */
namespace MyLogger {
	template<>
	struct user_impl<MyStruct> {
		static std::string Run(const MyStruct& value) {
			/* Convert each member to string (either using the same library or some other way) */
			auto a_str = MyLogger::Log(value.a); /* This will use library_impl<int> */
			auto b_str = MyLogger::Log(value.b); /* This will use user_impl<bool> */
			auto c_str = MyLogger::Log(value.c); /* This will use fallback_impl<float> */
			return "MyStruct(" + a_str + ", " + b_str + ", " + c_str + ")";
		}
	};
}

int main() {
	int value = 42;
	std::cout << "my value: " << MyLogger::Log(value) << std::endl; // Will use library_impl<int>

	bool flag = true;
	std::cout << "my flag: " << MyLogger::Log(flag) << std::endl; // Will use user_impl<bool>

	float floatValue = 3.14f;
	std::cout << "my float value: " << MyLogger::Log(floatValue) << std::endl; // Will use fallback_impl<float>

	MyStruct myStruct{ 1, false, 2.718f };
	std::cout << "my struct: " << MyLogger::Log(myStruct) << std::endl; // Will use user_impl<MyStruct>

	//wait for user input before exiting
	std::cout << "Press Enter to exit...";
	std::cin.get();

	return 0;
}