# (std-like) Future

## Пререквизиты

- [runtime/thread_pool](/tasks/runtime/thread_pool)

---

В этой задаче мы напишем первые (наивные) фьючи для пула потоков, взяв за образец [`std::future`](https://en.cppreference.com/w/cpp/thread/future).

## Future ← Promise

Пара (`Future<T>`, `Promise<T>`) образует одноразовый канал для возврата результата из асинхронной операции.

Результат – значение типа `T` или исключение.

С каналом работают два потока:
- _Producer_ – поток, выполняющий асинхронную операцию
- _Consumer_ – поток, запускающий асинхронную операцию и потребляющий ее результат

Producer и consumer – это _роли_ потоков, один поток может совмещать разные роли (работая с разными каналами).

Канал – одноразовый (_one-shot_): передать можно только один результат.

`Promise<T>` – конец канала для записи, предоставляет producer-у два метода:
- `SetValue` для передачи значения и
- `SetException` для передачи исключения (в виде [`std::exception_ptr`](https://en.cppreference.com/w/cpp/error/exception_ptr)).

`Future<T>` – конец канала для чтения, предоставляет consumer-у единственный метод `Get`, который блокирует поток до тех пор, пока через `Promise` не будет отправлен результат.  

### Пример

```cpp
ThreadPool pool{/*threads=*/4};
pool.Start();

// Создаем "канал"
Promise<int> p;
auto f = p.MakeFuture();

pool.Submit([p = std::move(p)]() mutable {
  std::this_thread::sleep_for(3s);  // <-- Выполняем полезную работу
  p.SetValue(42);  // <-- "Возвращаем" значение
});

int v = f.Get();  // <-- Дожидаемся значения
fmt::println("v = {}", v);

pool.Stop();
```

### Lifetimes 

Асинхронность приводит к несогласованности лайфтаймов:

- Consumer может не дожидаться значения на `Future`, вообще не вызывать метод `Get`. Так что `Future` может быть разрушена до того, как producer отправит результат через `Promise`.

- Producer может отправить результат и разрушить `Promise` до того, как consumer вызовет `Get` на `Future`.

## Указания по реализации

Используйте [`std::expected<T, E>`](https://en.cppreference.com/w/cpp/utility/expected).

В этой задаче вы можете создавать новые файлы в директории `exe`.
