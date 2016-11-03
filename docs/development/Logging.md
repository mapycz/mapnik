Mapnik >= 2.1.x supports a variety of logging options to make it possible to see what is happening under the hood.

## Overview

The logging system has concepts that need to be understood by both users who want to turn on or off logging at runtime and by developers seeking to debug their code and offer good logging possibilities for user.

The total possible logging functionality is determined at compile time, but the logging verbosity can also be controlled at runtime.

## Concepts

### Severity levels

Severity levels are the way Mapnik categorizes logging messages. Each logging message that a Mapnik developer adds to the codebase will have a severity level.

Here are all the macros available to Mapnik C++ developers:

```cpp
#include <mapnik/debug.hpp>

// Output a string in WARN severity level
MAPNIK_LOG_WARN(object_name) << "This is a warning message";

// Output a string in DEBUG severity level
MAPNIK_LOG_DEBUG(object_name) << "This is a debug message";

// Output a string in ERROR severity level
MAPNIK_LOG_ERROR(object_name) << "This is an error message";
```

Also, at runtime, severity level can be used by users to control verbosity.

For example if a user wants to disable all log output, here is how that would be done in python:

```python
import mapnik
mapnik.logger.set_severity(mapnik.severity_type.None)
```

Statements that use `MAPNIK_LOG_ERROR` inside the Mapnik code, will be always be logged if Mapnik is compiled with the default settings and the runtime settings are unchanged. This is because the `DEFAULT_LOG_SEVERITY` option is set to `error` at compile time and therefore the `mapnik.severity_type` (for example in python) is `mapnik.severity_type.Error`. Besides the `error` level, there are three other named levels: `debug`, `warn`, and `none`. The `none` level disables all logging and ensures Mapnik is completely silent. The `warn` level will trigger logging messages that give more verbose information on what Mapnik is doing and is designed to help users understand program behavior. When `warn` is enabled users will see both `error` and `warn` level severity messages logged. The last level is `debug` which reveals all possible logging messages and is designed for developers to reveal possible bugs at runtime or for users to see all possible information to provide that in bug reports to developers.

### Error level vs Exceptions

Mapnik supports exceptions in the C++ API and its bindings like python and node.js. Exceptions are throw when an error is encountered that should be handled by the application calling Mapnik. Mapnik basically is saying, this happened (the exception) and you need to do something about it.

The Error level of logging is different. It can be used along with exceptions to output more contextual information about what caused the exception. But the primary intended usage is for when throwing an exception is not warranted but is it still important to communicate that something happened during program execution that was either unexpected or not optimal. For example, when rendering a map if a png symbol referenced by a style is not found it does not make sense to stop rendering altogether by throwing an exception. Instead rendering continues till the end but the missing symbol file path will be logged using the `error` level.

### Logging vs. Statistics

Here we refer to "Logging" as messages printed to output that describe what Mapnik is doing, including both things that could be considered unexpected errors and normal execution that might be valuable to know happened.

Statistics are also messages printed to output, but are specifically for reporting how much time certain operations took to execute.

It is possible to control logging messages using a combination of compile time and runtime options. Statistic messages on enabled at compile time and cannot be turned off at runtime.

## Compile time options

When you configure Mapnik you have several options that can be set during the configure stage of building which impact how logging behaves.

- ENABLE_STATS - Default `False`. If set to `True` then timing output will be printed to `stderr` of a variety of performance critical code paths. As of Mapnik 2.2 only datasource plugins respond to this option by providing progress timers. In the future more performance critical code paths will gain progress timers. Feel free to create an issue to request more specific coverage.
- ENABLE_LOG - This option controls which severity levels should be compiled into the Mapnik binary. The default value is `False`. This default means that not all severity types will be compiled into Mapnik: only `error` and `none` levels will be available to toggle at runtime. The reason for this is to ensure that Mapnik runs fast by default, because verbose logging can slow down code execution. If set to `True` then logging of all severity types will be enabled and available at runtime. NOTE: This option defaults to `True` if `DEBUG=True` (see option below).
- DEFAULT_LOG_SEVERITY - The `DEFAULT_LOG_SEVERITY` option controls which severity level will be used by default at runtime - the value it will be initialized to at startup. It is not recommended to change this option at configuration time, unless you are a developer or doing customized packaging of Mapnik. Users can change the severity level easily at runtime. The default level is `error`. Other named severity levels are: `debug`, `warn`, `none`. An important caveat is that the `ENABLE_LOG` option controls which severity levels are compiled into Mapnik by default, so setting `DEFAULT_LOG_SEVERITY` to `warn` or `debug` will have no effect unless `ENABLE_LOG` is `True`.
- DEBUG - Default `False`. This flag adds various compile flags that add debugging information to the Mapnik binary. Mapnik runs slower and the binary will be much larger when build in debug mode than release mode (`DEBUG=False`). For this reason - that performance is not critical if you are building in debug mode - `ENABLE-LOG` is set to `True` in this mode to ensure that all possible logging verbosity is available.
- RENDERING_STATS - This is an experimental option that may be removed in future releases. It defaults to `False`, but can be set to `True` to enable verbose logging of rendering behavior providing the timing of style and layer processing.

### Examples

To compile Mapnik in release mode but will all logging severity levels enabled do:

```sh
./configure ENABLE_LOG=True DEBUG=False
```

To compile Mapnik in release mode but will all logging severity levels and performance stats enable:

```sh
./configure ENABLE_LOG=True DEBUG=False ENABLE_STATS=True
```

To compile Mapnik such that it is completely silent by default:

```sh
./configure DEFAULT_LOG_SEVERITY=none ENABLE_LOG=False DEBUG=False ENABLE_STATS=False
```

NOTE: options are cached in the `config.py` file. So, make sure to override any options that you previously used, or remove them from your `config.py` file.

## The runtime logger

At runtime Mapnik can be told to log more or less or not at all. For example, in python if you want all logging to be suppressed you can do:

```python
import mapnik
mapnik.logger.set_severity(mapnik.severity_type.None)
```

Or, if you want to set it back to the default setting do:

```python
mapnik.logger.set_severity(mapnik.severity_type.Error)
```

Or, if your Mapnik build has been compiled with `ENABLE_LOG=True` then you can enable `debug` or `warn` levels:

```python
mapnik.logger.set_severity(mapnik.severity_type.Warn)
# or
mapnik.logger.set_severity(mapnik.severity_type.Debug)
```
In C++ to use the runtime logger from your program first include this header:

```cpp
#include <mapnik/debug.hpp>
```
Then in your C++ code you could disable all logging by doing:

```
mapnik::logger::instance().set_severity(mapnik::logger::none);
```

## Developer Best practices
When `ENABLE_LOG` is set to `False` the logging framework optimizes away `Warn` and `Debug` logging macros leaving in place all `Error` macros, since these are considered useful enough to be show no matter what and not likely to harm performance.

But if you want to put logging in performance critical code or need to perform complex code before outputting a string to debug, then it's better to disable the macro manually to avoid the chance that it adds overhead even if not used. The way to do this is to wrap the entire block of code in `#ifdef MAPNIK_LOG`.

Here is an example:

```cpp
#include <mapnik/debug.hpp>
...
#ifdef MAPNIK_LOG
  // here we need to output a string that will be output in DEBUG severity level:
  const double a = 1.0 / sin(x);
  const double z = a * connection->get_zoom_from_sql_call();
  MAPNIK_LOG_DEBUG(cairo_renderer) << "Log the variable " << z << ", visible at DEBUG severity level";
#endif
```