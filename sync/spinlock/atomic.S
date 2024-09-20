#if(APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.globl FUNCTION_NAME(AtomicLoad)
.globl FUNCTION_NAME(AtomicStore)
.globl FUNCTION_NAME(AtomicExchange)

FUNCTION_NAME(AtomicLoad):
    movq    (%rdi), %rax
    ret

FUNCTION_NAME(AtomicStore):
    movq    %rsi, (%rdi)
    ret

FUNCTION_NAME(AtomicExchange):
    xchgq (%rdi), %rsi
    movq %rsi, %rax  # Store
    retq
