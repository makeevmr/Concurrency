#pragma once

// Relax in favour of the CPU owning the lock
// https://c9x.me/x86/html/file_module_x86_id_232.html

inline void SpinLoopHint() {
  asm volatile("pause\n" : : : "memory");
}
