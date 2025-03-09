# TryLock для TicketLock

Реализуйте метод `TryLock` для [`TicketLock`](source/ticket_lock.hpp).

Семантика `TryLock`:

* Если спинлок свободен, то захватить его **без ожидания** и вернуть `true`
* Если спинлок захвачен другим потоком, и текущему потоку нужно ждать освобождения блокировки, то **без ожидания** вернуть `false`

Если вызов `TryLock()` вернул `true`, то поток захватил спинлок и находится в критической секции.

Каждый поток, прошедший через `Lock` или успешный `TryLock`, должен получать от спинлока уникальный порядковый номер.

Вызов метода `TryLock` должен завершаться за конечное число шагов, которое не зависит от числа потоков, разрядности машинного слова и т.п. В частности, в реализации `TryLock` нельзя вызывать метод `Lock`: число итераций в цикле ожидания в `Lock` зависит от числа потоков в очереди перед нами.

Неудачные попытки `TryLock` не должны приводить к вечным блокировкам вызовов `Lock`.

---

В решении вы можете использовать любые атомарные RMW-операции, которые есть у [std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic).

Реализацию методов `Lock` и `Unlock` менять нельзя.

## Формализация

Попробуйте описать семантику `TryLock` формально. 

Что значит «_если спинлок свободен_»? Про какой момент идет речь?

### References

- https://jepsen.io/consistency/models/linearizable
- [Linearizability: A Correctness Condition for Concurrent Objects](https://cs.brown.edu/~mph/HerlihyW90/p463-herlihy.pdf)

## Weak MM

[После формализации семантики `TryLock`]

Может ли `TryLock` соврать пользователю, что спинлок захвачен, хотя на самом деле он свободен?

Изучите [гарантии](https://eel.is/c++draft/thread.mutex.requirements#mutex.general-15) `std::mutex::try_lock`.

Почему так? Ответ – сложный: [Foundations of the C++ Concurrency Memory Model](https://rsim.cs.uiuc.edu/Pubs/08PLDI.pdf)
