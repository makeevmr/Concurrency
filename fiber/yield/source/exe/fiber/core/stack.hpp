#pragma once

#include <twist/ed/sure/stack/mmap.hpp>

namespace exe::fiber {

using Stack = twist::ed::sure::stack::GuardedMmapStack;

}  // namespace exe::fiber
