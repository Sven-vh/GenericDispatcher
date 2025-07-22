#pragma once
#include <type_traits>
#include <utility>

namespace svh {

	/* Simple helper for clean error handling */
	template<class T>
	struct always_false : std::false_type {};

	/* SFINAE-detection, is true when Run is well formatted */
	template< template<class, class...> class Provider, class T, class... Args>
	struct has_impl {
	private:
		// try to call Provider<T,Args...>::Run(v,args...)
		template<class U>
		static auto test(int)
			-> decltype(
				Provider<U, Args...>::Run(
					std::declval<const U&>(),
					std::declval<Args>()...
				),
				std::true_type{}
				);

		// fallback if substitution failed
		template<class>
		static std::false_type test(...);

	public:
		static constexpr bool value = decltype(test<T>(0))::value;
	};

	template< template<class, class...> class Provider, class T, class... Args>
	constexpr bool has_impl_v = has_impl<Provider, T, Args...>::value;

	/* Tries every prodiver in given order to see if one of them is build correct*/
	template< template<class, class...> class... Providers >
	struct AnyDispatcher;

	/* Default fallback, if no provider works */
	template<>
	struct AnyDispatcher<> {
		template<class T, class... Args>
		static auto Run(const T&, Args&&...) {
			static_assert(always_false<T>::value,
				"No matching provider for this type/args!");
		}
	};

	// Recursivly tries each provider in the list
	template<
		template<class, class...> class First,
		template<class, class...> class... Rest
	>
	struct AnyDispatcher<First, Rest...> {
		template<class T, class... Args>
		static auto Run(const T& v, Args&&... args) {
			if constexpr (has_impl_v<First, T, Args...>) {
				// dispatch to First::Run
				return First<T, Args...>::Run(v, std::forward<Args>(args)...);
			} else {
				// try the next one
				return AnyDispatcher<Rest...>::Run(v, std::forward<Args>(args)...);
			}
		}
	};
} // namespace svh
