# godoctopus2 Unit Tests

This directory contains unit tests for the godoctopus2 C++ module using Godot's doctest framework.

## Overview

The godoctopus2 module implements the gameplay runtime for Robot Odyssey in a custom Godot engine fork. This test suite covers:

- **GameNode** - The core Godot Node that initializes and manages the ECS world
- **Trigger Systems** - Rune effects, buff systems, and gameplay triggers declared in `TriggerDeclaration.cpp`
- **Type System** - Verification that C++ types are properly registered with Godot

## Test Files

## Building Tests

From `robot_odyssey-godot/`:

```bash
# Build engine with tests enabled
scons tests=yes

# Run all tests
./bin/godot.linuxbsd.editor.x86_64 --test

# Run only godoctopus2 tests
./bin/godot.linuxbsd.editor.x86_64 --test --test-suite="*[godoctopus2]*"

# Run with verbose output
./bin/godot.linuxbsd.editor.x86_64 --test --success
```

## Test Framework

Tests use Godot's integrated **doctest** framework. Key macros:

- `TEST_CASE(description)` - Define a test case
- `SUBCASE(description)` - Create variations within a test (re-runs test setup for each)
- `CHECK(condition)` - Assert condition (test continues on failure)
- `REQUIRE(condition)` - Assert condition (test fails immediately on failure)
- `CHECK_MESSAGE(condition, message)` - Assert with custom message

## References

- **Godot Testing Documentation**: https://docs.godotengine.org/en/stable/engine_details/architecture/unit_testing.html
- **TriggerDeclaration.cpp**: Defines all buff and trigger systems tested here
- **octopus2 Tests**: Reference C++ tests at `robot_odyssey-godot/modules/godoctopus2/src/octopus2/src/octopus/test/`
