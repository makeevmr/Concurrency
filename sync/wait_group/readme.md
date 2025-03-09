# WaitGroup

`WaitGroup` – счетчик абстрактной «работы».

Изначально в счетчике 0.

Операции:
- `void Add(size_t count)` – добавить положительный `count`
- `void Done()` – вычесть единицу
- `void Wait()` – заблокировать текущий поток до тех пор, пока значение счетчика не опустится до нуля

## Примеры

Будем использовать `WaitGroup` для ожидания запланированных в `ThreadPool` задач:

### Пример 1

```cpp
void ThreadPoolExample() {
  ThreadPool pool{4};
  pool.Start();
  
  std::atomic<uint64_t> work{0};
  
  WaitGroup wg;
  
  for (size_t i = 0; i < 256; ++i) {
    wg.Add(1);
    
    pool.Submit([&] {
      work.fetch_add(1);
      wg.Done();
    });
  }
  
  // <- К этому моменту завершены все вызовы Add
  wg.Wait();  // Блокируем поток до выполнения всех задач в пуле
  
  fmt::println("Work = {}", work.load());
  
  pool.Stop();
}
```

### Пример 2

```cpp
void ConcurrentAddExample() {
  ThreadPool pool{4};
  pool.Start();
  
  WaitGroup wg;
  
  wg.Add(1);
  pool.Submit([&wg] {
    wg.Add(1);  // <- Этот вызов Add конкурирует с вызовом Wait
    ThreadPool::Current()->Submit([&wg] {
      wg.Done();
    });
    wg.Done();
  });
  
  wg.Wait();  // Блокируем поток до выполнения _обеих_ задач
  
  pool.Stop();
}
```

### Пример 3

```cpp
void CyclicExample() {
  ThreadPool pool{4};
  pool.Start();
  
  WaitGroup wg;
  
  for (size_t i = 0; i < 256; ++i) {
    wg.Add(1);
    pool.Submit([&wg] {
      wg.Done();
    })
  };
   
  wg.Wait();

  // Используем WaitGroup повторно
  
  wg.Add(1);  // Новые Add должны быть упорядочены с Wait прошлой "эпохи"
  pool.Submit([&wg] {
    wg.Add(1);
    ThreadPool::Current()->Submit([&wg] {
      wg.Done();
    })
    wg.Done();
  });
  
  wg.Wait();
  
  pool.Stop();
}
```

## Go

Идея примитива взята из языка Go: 
- [sync.WaitGroup](https://pkg.go.dev/sync#WaitGroup)
- [Go by Example: WaitGroups](https://gobyexample.com/waitgroups)

## Правила

Зафиксируем правила работы с `WaitGroup`:

Вызовы `Add` на нулевом счетчике должны быть упорядочены (формально – частичным порядком [_happens-before_](https://eel.is/c++draft/intro.races#def:happens_before)) до `Wait` пользователем `WaitGroup` .

Вызовы `Done` и остальные вызовы `Add` при этом могут (и будут) конкурировать с `Wait`.

Пользователь должен гарантировать, что значение счетчика не опускается ниже нуля.

Вызывать `Wait` можно из нескольких потоков.

`WaitGroup` можно использовать повторно, т.е. добавлять в него новую «работу» после завершения вызова `Wait`. 
В таком сценарии новые `Add` должны быть упорядочены (в _happens-before_) после `Wait` самим пользователем `WaitGroup`.

См. примеры выше и документацию [sync.WaitGroup](https://pkg.go.dev/sync#WaitGroup).

## Задание

Реализуйте [`WaitGroup`](source/wait_group.hpp)
