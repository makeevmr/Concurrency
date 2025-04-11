# Fibers, Oh My!

## Пререквизиты

- [runtime/thread_pool](/tasks/runtime/thread_pool)
- [fiber/coroutine](/tasks/fiber/coroutine)

---

В этой задаче мы напишем многопоточные кооперативные файберы.

Планировщик для файберов нами [уже реализован](/tasks/runtime/thread_pool), это пул потоков.

Механизм остановки / возобновления исполнения – [тоже](/tasks/fiber/coroutine), это stackful корутина.

Остается лишь скомбинировать их!

## Пример

Результат будет выглядеть так:

```cpp
void FibersExample() {
  // В этой задаче мы начнем писать фреймворк для concurrency
  // Он будет называться `exe` (от execution)
  using namespace exe;

  // Пул потоков будет служить планировщиком для файберов
  runtime::ThreadPool scheduler{4};
  scheduler.Start();
  
  thread::WaitGroup wg;

  for (size_t i = 0; i < 128; ++i) {
    wg.Add(1);
    // Планируем файбер в пул потоков
    fiber::Go(scheduler, [&wg] {
      // https://go.dev/tour/flowcontrol/12
      Defer defer([&wg] {
        wg.Done();
      });
      
      for (size_t j = 0; j < 1024; ++j) {
        // Уступаем поток планировщика другому файберу
        fiber::Yield();
      }
    });
  }
  
  // Файберы исполняются _параллельно_ на потоках пула

  // Дожидаемся завершения файберов
  wg.Wait();
  
  // Выключаем планировщик
  scheduler.Stop();
}  
```

См. [play/main.cpp](play/main.cpp)

## Scheduling

В этой задаче мы поддержим лишь базовые [системные вызовы](source/exe/fiber/sched) про планирование:

### `Go`

Заголовок: [`fiber/sched/go.hpp`](source/exe/fiber/sched/go.hpp)

`Go(Scheduler&, Body)` – запланировать процедуру пользователя на исполнение в новом файбере.

`Go(Body)` – (precondition: вызывается **из файбера**) запланировать процедуру пользователя на исполнение в новом файбере **в текущий планировщик**.

### `Yield`

Заголовок: [`fiber/sched/yield.hpp`](source/exe/fiber/sched/yield.hpp)

`Yield()` – перепланировать текущий файбер, уступить поток планировщика другим файберам.

~ [Gosched](https://pkg.go.dev/runtime#Gosched) в Golang. 

## Дизайн

_Файбер_ = _Корутина_ × _Планировщик_

Компоненты в этой формуле **ортогональны**:

- _Планировщик_ (пул потоков) не знает про файберы и корутины, его единственная ответственность – исполнять _задачи_. 
- _Корутина_ не знает про потоки, задачи и планировщики, она лишь реализует suspendable процедуру.

Реализация файберов **комбинирует** эти компоненты:
- генерирует служебные задачи для планировщика,
- возобновляет и останавливает в них корутины, которые исполняют процедуры пользователей.

С точки зрения планировщика файбер = цепочка задач.

С точки зрения корутин файбер = корутина + правила остановки и возобновления (кооперативная многозадачность).

## Exe

```sh
exe
├── fiber # Кооперативные файберы
│   ├── sched # Планирование (Go, Yield, SleepFor, ...)
│   ├── sync # Примитивы синхронизации (Mutex, Event, WaitGroup, ...)
├── runtime # Среда исполнения (пока - просто ThreadPool)
├── thread # Примитивы синхронизации для потоков
└── util # Defer, ...
```

## References

- [Fibers, Oh My!](https://graphitemaster.github.io/fibers/)
- [Fibers and Continuations for the Java Virtual Machine](https://cr.openjdk.java.net/~rpressler/loom/Loom-Proposal.html), [Ron Pressler and Alan Bateman – Project Loom](https://www.youtube.com/watch?v=J31o0ZMQEnI)

## Задание

1) [Перенесите](source/exe/runtime/thread_pool.hpp) реализацию пула потоков из задачи [runtime/thread_pool](/tasks/runtime/thread_pool)
2) [Перенесите](source/exe/fiber/core/coroutine.hpp) реализацию корутины из задачи [fiber/coroutine](/tasks/fiber/coroutine)
3) Через `ThreadPool` и `Coroutine` выразите [файберы](source/exe/fiber/)

Код эволюционирует, так что по пути потребуются косметические изменения: нужно будет двигать файлы, корректировать инклюды, оборачивать код в дополнительные пространства имен и т.п.

## Замечания по реализации

### Файберы

#### Гигиена

Класс `Fiber` не должен быть виден пользователю через [заголовки `sched`](source/exe/fiber/sched/).

### Корутина

#### `SuspendContext`

Мы будем применять корутины только для реализации файберов и не предполагаем 
их прямого использования клиентом фреймворка.

Как следствие, нам не пригодится `SuspendContext`, будет достаточно прямого вызова `Suspend` на `Coroutine`.

#### Twist

Используйте [Twist-_ed_ версии](https://gitlab.com/Lipovsky/twist/-/blob/master/docs/ru/twist/ed/sure.md?ref_type=heads) `ExecutionContext` и стеков.
