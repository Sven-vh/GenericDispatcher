![Generic Dispatcher Example image](https://github.com/Sven-vh/GenericDispatcher/blob/main/images/GenericDispatcherExample.png)

# Generic Dispatcher

A system that helps me make scalable libraries. It allows me to have a single entry point and multiple implementations.

## Example

For complete example, see the [``solution/Main.cpp``](https://github.com/Sven-vh/GenericDispatcher/blob/main/svh/GenericDispatcher.hpp).

Below I go in more detail about how I make a Logger with the generic dispatcher.

We first define implementations that we want to dispatch to. This can be as many as we want. In this example I define 3 implementations. One for the `user` that uses the library, one for the `library` itself, and a `fallback` implementation that is used when no `user` or `library` implementation is available.

```cpp
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
```

> [!NOTE]
> This can be as many implementations as you want.
> 
> Also, the ``user_impl`` and ``library_impl`` are empty structs. They are used to identify the implementation type. The actual implementation is done in the `Run` function somewhere else.

Next we define a dispatcher. <u> Order Matters!</u><br>
The order in which you define the implementations in the dispatcher is important. The dispatcher will try to find the first implementation that matches the type and arguments provided.

```cpp
using LoggerDispatcher = svh::AnyDispatcher<
    user_impl,
    library_impl,
    fallback_impl
>;
```

We can then use the dispatcher to call the `Run` function with the type and arguments we want to dispatch to.

```cpp
template<typename T, typename... Args>
static std::string Log(const T& value, Args&&... args) {
    return LoggerDispatcher::Run(value, std::forward<Args>(args)...);
}
```

## Usage

With the example above, we can now make an implementation for any type we want. Since I'm the library maker of ``MyLogger``, I can provide a bunch of implementations for common types like `int`, `float`, `std::string`, etc.

```cpp
/* Library implementations */
template<>
struct MyLogger::library_impl<int> {
    static std::string Run(const int& value) {
        return std::to_string(value);
    }
};

template<>
struct MyLogger::library_impl<float> {
    static std::string Run(const float& value) {
        return std::to_string(value);
    }
};

template<>
struct MyLogger::library_impl<bool> {
    static std::string Run(const bool& value) {
        return value ? "true" : "false";
    }
};
```

The user that is using my library can then provide their own implementations for the types they want to log. This allows them to customize the logging behavior without modifying the library itself.

```cpp
struct MyStruct {
    int a;
    float b;
    bool c;
};

template<>
struct MyLogger::user_impl<MyStruct> {
    static std::string Run(const MyStruct& value) {
        auto a_str = std::to_string(value.a);
        auto b_str = std::to_string(value.b);
        auto c_str = value.c ? "yes" : "no";
        /* Or */
        /*
        auto a_str = MyLogger::Log(value.a);
        auto b_str = MyLogger::Log(value.b);
        auto c_str = MyLogger::Log(value.c);
        */
        return "MyStruct { a: " + a_str + ", b: " + b_str + ", c: " + c_str + " }";
    }
};
```

So then when the user calls
```cpp
MyLogger::Log(MyStruct{1, 2.0f, true});
```

It will dispatch to the `user_impl<MyStruct>` implementation, which will format the struct as a string. And return:

```
MyStruct { a: 1, b: 2.000000, c: yes }
```

## Overriding 
So if (me) the library maker, has implemented an implementation for `bool`, like so:

```cpp
/* Library implementations */
template<>
struct MyLogger::library_impl<bool> {
    static std::string Run(const bool& value) {
        return value ? "true" : "false";
    }
};
```

The user can "override" this implementation by providing their own implementation for `bool`:

```cpp
template<>
struct MyLogger::user_impl<bool> {
    static std::string Run(const bool& value) {
        return value ? "yes" : "no";
    }
};
```

This is possible because the dispatcher will first look for a `user_impl` implementation, meaning it will find the `user_impl<bool>` implementation before the `library_impl<bool>` implementation.

This is especially useful when I, the library maker, makes a generic implementation for a type, but the user wants to customize the logging behavior for that type. For example, every type that has a `std::to_string` implementation will go to this implementation:

```cpp
template <typename T, typename = void>
struct has_to_string : std::false_type {};

template <typename T>
struct has_to_string<T, std::void_t<decltype(std::to_string(std::declval<T>()))>> 
    : std::true_type {};

template <typename T>
constexpr bool has_to_string_v = has_to_string<T>::value;

template<typename T>
struct MyLogger::library_impl<T, std::enable_if_t<has_to_string_v<T>>> {
    static std::string Run(const T& value) {
        return std::to_string(value);
    }
};
```

But maybe the user wants to log an ``int`` as a hex value, so they can provide their own implementation:

```cpp
template<>
struct MyLogger::user_impl<int> {
    static std::string Run(const int& value) {
        std::stringstream ss;
        ss << std::hex << value;
        return ss.str();
    }
};
```

Then everything else will still use the `library_impl<int>` implementation, but the ``int`` type will use the `user_impl<int>` implementation.

## Extra Arguments

The dispatcher also supports extra arguments. This allows you to pass additional information to the implementation. For example:

```cpp
template<>
struct MyLogger::user_impl<int, bool> {
    static std::string Run(const int& value, const bool& hex) {
        if (hex) {
            std::stringstream ss;
            ss << std::hex << value;
            return ss.str();
        } else {
            return std::to_string(value);
        }
    }
};
```

Then you can call the `Log` function with an additional argument:

```cpp
MyLogger::Log(42, true); // Will log as hex
MyLogger::Log(42, false); // Will log as decimal
```

## Conclusion

I plan on using this method for multiple libraries/systems. Like:
- Logging
- Serialization, see [svh/prefabs](https://github.com/Sven-vh/prefabs) (WIP)
- ImGui Inspector
- Registering for scripting languages
