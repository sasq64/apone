
## Tween engine


```c++
	Tween::make().to(position, vec2(5,5)).to(alpha, 1.0).seconds(1.5).onComplete([=]() {
		removeObject();
	});
```

```c++
Tween make_tween()
```
Creates a tween object.

### Tween Member functions

```c++
Tween& to(T &target, T value, int cycles = 1)
```

Add an interpolation to the tween so that the variable `target` will interpolate from it's current value to the given 'value'.

```c++
Tween& from(T &target, T value)
```

Add an interpolation to the tween so that the variable `target` will interpolate from the given `value` to it's current value. 

```c++
Tween& on_complete(function<void()> f)
```

Set a function to be called when the tween is done.

```c++
Tween& seconds(float s)
```

Set the number of seconds that this tween (all interpolations) will run for.

```c++
Tween& delay(float d)
```

Set a delay (in seconds) for any interpolations that are subsequently added.

```c++
Tween& linear()
Tween& smoothstep()
Tween& easeinback()
Tween& easeoutback()
Tween& easeinsine()
Tween& easeoutsine()
```

Set the tweening function used to tween any interpolations that are subsequently added.

