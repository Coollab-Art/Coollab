
### Debug Options


### Never use std::filesystem functions directly, use Cool::File instead

We wrap the std::filesystem functions and handle exceptions.
Note that using the types are fine (e.g. you can freely use `std::filesystem::path`)

### Use trailing return types

:::danger Don't
```cpp
bool do_something();
```
:::

:::tip Do
```cpp
auto do_something() -> bool;
```
:::

:::tip Do
```cpp
void do_something();
```
:::

Reason: consistency, this is what we do everwhere in the code. Also I like this syntax more, I find it more readable (avoids very long types in front of the function, makes it easier to read a list of methods in a class, all names are nicely alined;

)