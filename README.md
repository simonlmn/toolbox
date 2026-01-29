# toolbox: Collection of common utilities and functions.

General purpose library with functions and classes usable in many projects, covering c-string handling, conversions and in- or output.

## Overview

This library provides a small set of lightweight utilities tailored for Arduino-style environments. The focus is on minimal allocations, explicit buffer usage, and support for strings stored in RAM or PROGMEM.

## Provided functionality

- **String handling**: `strref` for read-only string views across RAM/PROGMEM/Arduino `String`, plus `str<N>` as a fixed-size string buffer.
- **Formatting**: `Formatter` and `format()` helpers for safe, bounded `printf`-style formatting into user-managed buffers.
- **Optional values**: `Maybe<T>` as a compact alternative to `std::optional`, with basic combinators.
- **Conversions**: `convert<T>` for parsing and formatting common primitive types, plus boolean format variants.
- **Decimal numbers**: `Decimal` for fixed-point style decimal I/O backed by a 64-bit integer.
- **Fixed-capacity map**: `FixedCapacityMap<K, V, N>` for sorted key/value storage with deterministic memory usage.
- **Streams**: Minimal `IInput`/`IOutput` interfaces, string- and stream-backed adapters, and `InputStream` for bridging to Arduino `Stream` APIs.
- **Repository transactions**: `Transaction<R>` and `beginTransaction()` helpers for simple commit/rollback patterns.

## Notes

- Some conversions and formatting APIs reuse internal buffers; treat returned strings as transient unless you provide your own buffer.
- PROGMEM strings are supported through `strref` to avoid redundant copies.
