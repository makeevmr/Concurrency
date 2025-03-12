# Condvar

## Пререквизиты

- [sync/mutex](/tasks/sync/mutex)
- Рекомендуется
  - [sync/barrier](/tasks/sync/barrier)
  - [sync/semaphore](/tasks/sync/semaphore)
  - [sync/wait_group](/tasks/sync/wait_group)

---

В этой задаче мы напишем [`std::condition_variable`](https://en.cppreference.com/w/cpp/thread/condition_variable).

## Кондвары

_Condition variable_ или просто _condvar_ (_кондвар_) – примитив синхронизации, который позволяет потокам ждать наступления событий. 

Под событием в контексте кондваров понимается модификация разделяемого состояния, защищенного мьютексом (например, `Push` задачи в очередь пула потоков).

## Операции

У кондвара есть один метод для ожидания (`Wait`) и два метода для уведомления (`NotifyOne` и `NotifyAll`).

Метод `Wait` можно вызывать только внутри критической секции, в него передается захваченный мьютекс.

Семантика `cv.Wait(mutex)` –

1. **Aтомарно** 
   a) отпустить `mutex` и 
   b) встать в очередь ожидания (кондвара) нотификации от `cv.NotifyOne()` или `cv.NotifyAll()`.
2. После пробуждения захватить обратно отпущенный `mutex` и завершить вызов.

Таким образом, до начала и после завершения вызова `Wait` поток владеет мьютексом, внутри вызова – отдает владение.

Семантика `cv.NotifyOne()` / `cv.NotifyAll()` – разбудить один из потоков / все потоки, стоящие в очереди ожидания в вызове `Wait`.

Если при выполнении `NotifyOne` или `NotifyAll` ни один поток не ждал внутри вызова `Wait`, то нотификация будет пропущена, на будущие вызовы `Wait` она не повлияет.

### Futex

Можно сказать, что кондвар обобщает фьютекс: ждать можно на произвольном состоянии / произвольного предиката, а не только на одной ячейке памяти / зафиксированном условии ` == old`.

### Атомарность

Под атомарностью на шаге 1 метода `Wait` следует понимать атомарность **относительно** `NotifyOne` / `NotifyAll`: 

Не допускается сценарий, когда поток в методе `Wait` уже отпустил мьютекс, но еще не встал в очередь ожидания кондвара, и в этот момент другой поток захватил мьютекс, изменил разделяемое состояние и вызвал `cv.NotifyOne()`, но никого не разбудил.

См. также комментарий про атомарность в https://linux.die.net/man/3/pthread_cond_wait

### Spurious Wakeups

Метод `Wait` допускает *ложные пробуждения* (*spurious wakeups*):
- Вызов `Wait` может вернуть управление даже без нотификации от других потоков
- После вызова `NotifyOne` из вызова `Wait` могут выйти несколько потоков

Найдите сценарий (или сценарии) ложного пробуждения в своей реализации.

Ложные пробуждения можно назвать дефектом реализации, но исправлять их и думать о них обычно<sup>†</sup> нет необходимости: типичные паттерны корректного использования кондваров автоматически учитывают и ложные пробуждения.

<sup>†</sup> Редкий пример исключения из этого правила вы встретите в одной из соседних задач.

## Задание

Перенесите реализацию `Mutex` из задачи [sync/mutex](/tasks/sync/mutex) в [`mutex.hpp`](source/mutex.hpp), мьютекс и кондвар будут тестироваться вместе.

Реализуйте `CondVar` из [`condvar.hpp`](source/condvar.hpp).
 
## Реализация

### Futex

Как и мьютекс, кондвар должен блокировать и будить ждущие потоки. А значит для реализации нам потребуется уже знакомый инструмент – системный вызов [`futex`](https://man7.org/linux/man-pages/man2/futex.2.html), с которым мы работаем через [`futex::Wait` из _Twist_](https://gitlab.com/Lipovsky/twist/-/blob/master/docs/ru/twist/ed/wait/futex.md?ref_type=heads).

### Fifo

Простая реализация кондвара через фьютекс не будет гарантировать порядок пробуждения потоков из `Wait`.

Нужна ли кондвару гарантия FIFO?

### ABA

Скорее всего ваша реализация будет подвержена [_ABA problem_](https://en.wikipedia.org/wiki/ABA_problem) из-за переполнения 32-битного счетчика.

Посмотрите, как ABA пытаются решить в [`pthread_cond_wait`](https://github.com/lattera/glibc/blob/895ef79e04a953cac1493863bcae29ad85657ee1/nptl/pthread_cond_wait.c#L193).

Возможны ли ложные пробуждения в этой реализации?

## In the wild

### Pthread

В `pthread_cond_wait` попытались гарантировать FIFO и решить ABA:

- [Описание](https://sourceware.org/bugzilla/show_bug.cgi?id=13165#c41)
- [Реализация](https://sourceware.org/git/?p=glibc.git;a=blob;f=nptl/pthread_cond_wait.c;h=2b434026c66ca3411746ea37cee3a9133f5d0172;hb=ed19993b5b0d05d62cc883571519a67dae481a14)
- [Презентация](https://wiki.linuxfoundation.org/_media/realtime/events/rt-summit2016/pthread-condvars-posix-compliance-and-the-pi-gap_darren-hart_torvald-riegel.pdf)

Но позже обнаружили в реализации [новый ABA](https://sourceware.org/bugzilla/show_bug.cgi?id=25847) 🤯

### Rust

Rust наоборот пошел по пути упрощения реализации: [Replace Linux Mutex and Condvar with futex based ones](https://github.com/rust-lang/rust/pull/95035)

## References

- https://en.cppreference.com/w/cpp/thread/condition_variable
- https://linux.die.net/man/3/pthread_cond_wait