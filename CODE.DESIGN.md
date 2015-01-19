
== Memory and pointers

* Arguments should be `shared_ptr<T>` for retained references or `const &T` for copies.

== Naming

* ClassName, methodName, variable_name, file_name.cpp

Build structure

== General

Use C++11 coding guidlines -- avoid *new* and *delete*, use values or shared pointers.

Be typesafe when possible. Use type erasure for storing generic data.

It's worth spending a lot of time on API interfaces.

Prefer `std::function` to interfaces for callbacks.

Use coreutils/callback for callbacks with variable number of arguments

Use exceptions for error conditions, but don't require them (Slow in Emscripten).

