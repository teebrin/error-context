# Error context

## Why

The C++ language provides exceptions to handle errors but, unlike Java and C#, the exception that is thrown is quite bare (for obvious performance reasons). It lacks a lot of information to help debugging an unexpected error that might occur.

This library allows attaching the stack trace and custom data to an exception when it is thrown, sacrificing some performance, to gather very detailed information when an exception occurs. Special care has been taken in the design to add zero overhead when no exceptions are thrown.  

### Existing solutions

#### Boost.Exception
The boost library provides a `boost::exception` to derive your own exceptions from. This has the disadvantage of adding a dependency to that boost library, and it won't work when using code that throws exceptions that are not derived from `boost::execption`, as is the case for the biggest part of the code that exists.

The library introduces a concept of `error_info` that is very interesting that has inspired this library's `Detail` concepts.

## How it works

### Using an error context

The idea is to provide an `error::Context` object where exceptions are being caught (right before the `try` statement) so that, when an exception occurs (if one does), a snapshot of the `StackTrace` is captured by this context. When an exception has been thrown, we can inspect that context to get the call stack that led to that exception getting thrown. For example:

```c++
int main(int argc, char** argv)
{
    // Will capture a stack trace if an exception gets thrown.
    y::error::Context context;
    
    try
    {
        functionThatMayThrow("something");
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception caught:\n  " << e << '\n'
                  << context << std::endl;
    }
    catch (...)
    {
        std::err << context << std::endl;
    }
    
    return 0;
}
```
If an exception occurs during execution of `functionThatMayThrow`, you will get an output that might look like:
```
Exception caught:
  std::runtime_error: Runtime error description
Contextual details:
  SomeDetail: something
Call stack on '' thread:
  __cxa_throw
  someOtherDeepFunction()
  functionThatMayThrow()
  main
  start
```
In this output, there are three sections:
* Type of the exception along with the text returned by `std::exception::what()`
* A list of custom details that can be added to the context as the stack unwinds (see later section)
* A list of nested function calls, up to the point where the exception has been thrown (up to 32 levels deep). Note that some intermediate functions will not appear if they have been inlined by the compiler (occurs often when enabling compiler optimizations).

### Adding details

When implementing a function that might throw an exception (directly but most likely indirectly), we can add some very specific custom details to the error context. To avoid any penalty when no exception gets thrown (hopefully most of the time), those details are added only when an exception occurs. Here is a simple example of a function that adds a custom detail when an exception occurs:

```c++
// Define a 'SomeDetail' detail type for which the value is kept as an std::string.
struct SomeDetail { using ValueType = std::string; };

void functionThatMayThrow(std::string_view argument)
try
{
    someOtherDeepFunction(argument);
}
catch (...)
{
    // Add as many details as needed to the context.
    // Most of the time, that will be some or all of the function arguments.
    y::error::Context::addDetail<SomeDetail>(argument);
    
    // Very important to rethrow the flying exception.
    throw;
}
```

Using this approach, details are added only when an exception is actually thrown. Only one detail is kept per detail type. That means that if many functions use the same detail type (like the `SomeDetail` type in the example), only the one that is the nearest from the exception `throw` location will be kept (the first to be added to the context since they are added as the stack is unwound).

Don't hesitate to define as many detail types as needed to ensure easy debugging. Of course, avoid adding details that should remain secret (such as passwords or crypto keys) since it is very likely that those details will be written somewhere, most likely in plain text!

The `ValueType` can be any type, including custom types. To allow outputting it, the `operator<<` should be defined when the operator is used with an `std::ostream&`.

### Avoid repeating your error handling code

For maximum debuggability, such a context can be used at many points:
- main() entry point
- thread entry points
- external request entry points

To avoid repeating the same code you saw in the first example above, a helper function is available to handle the context's lifetime, catching exceptions and calling an error handler. Let's revisit the main function shown earlier using that helper:

```c++
// Define how you want to handle an exception for your application
// so that it can be reused.
void showErrorAndExit(const y::error::Context& context, const std::exception* e)
{
    // e is null if the exception is not derived from std::exception.
    if (e) { std::cerr << "Exception caught:\n  " << *e << '\n'; }
    std::cerr << context << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
    // The helper function takes two arguments:
    // * A functor that is always called (the actual code to execute)
    // * A functor to call only when an exception is caught
    // Both functors should return the same type also returned by the helper.
    y::error::handleExceptionsWithContext(
            [] { functionThatMayThrow("something"); },
            &showErrorAndExit);
    
    return 0;
}
```

## Limitations

### Thread traversal

Since the error context is not attached to an exception instance, when such an exception crosses a thread boundary (using `std::async` for example), the context will not follow and will be lost/unavailable where the exception is ultimately caught and handled.

## Future

### StackTrace

Because `std::stacktrace` is very new (C++23) and not available to most C++ code, a non-standard, non-portable version of a `StackTrace` is provided within this library. When C++23 becomes more mainstream, it is planned to migrate towards the `std` version.

### Support for Windows

Platforms that are currently working:
* linux+gcc
* MacOS+clang

In the future, the following platform(s) might be supported:
* Windows+vc
* Windows+mingw-gcc

Pull-requests are welcomed!