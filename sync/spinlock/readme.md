# SpinLock

В этой задаче мы узнаем из чего сделаны [атомики](https://en.cppreference.com/w/cpp/atomic/atomic).

Вам дана [реализация простейшего Test-and-Set спинлока](spinlock.hpp), которая использует крафтовый [`Atomic`](atomic.hpp).

Но этому атомику не хватает реализации атомарных операций – `AtomicLoad`, `AtomicStore` и `AtomicExchange`.

Семантика этих операций такая же, как у `load`, `store` и `exchange` в `std::atomic<int64_t>`.

[Реализуйте их!](atomic.S)

А заодно заполните реализацию метода `TryLock` у спинлока.

## Asm

Реализовать атомики непосредственно на языке C++ невозможно, придется писать код на ассемблере. Реализация получится платформо-зависимой, в нашем случае это x86-64 + Linux.

## Memory Ordering

Обратите внимание: у операций [`std::atomic`](https://en.cppreference.com/w/cpp/atomic/atomic) из стандартной библиотеки есть дополнительный параметр – [`std::memory_order`](https://en.cppreference.com/w/cpp/atomic/memory_order).

В операцию `store` корректно передавать лишь некоторые из его значений: `relaxed`, `release` и `seq_cst`.

Каждому из этих значений в общем случае может соответствовать собственная реализация метода `store`.

### Вопросы

- `store` для какого `std::memory_order` вы реализовали в функции `AtomicStore`?
- А какая версия нужна спинлоку?

## Полезные ссылки

- [System V AMD64 ABI](https://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI)
- https://godbolt.org/ + обязательно установите флаг `-O2`.
- [x86 and AMD64 Instruction Reference](https://www.felixcloutier.com/x86/)
