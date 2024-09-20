#pragma once

#include <stdint.h>

typedef int64_t AtomicValue;
typedef volatile int64_t AtomicStorage;

// Atomic operations

// Atomically loads and returns the content of memory location 'loc'
AtomicValue AtomicLoad(AtomicStorage* loc);

// Atomically stores 'value' to memory location 'loc'
void AtomicStore(AtomicStorage* loc, AtomicValue value);

// Atomically replaces the content of memory location 'loc' with 'value',
// returns the content of the location before the call
AtomicValue AtomicExchange(AtomicStorage* loc, AtomicValue value);
