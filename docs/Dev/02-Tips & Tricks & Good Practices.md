
### Debug Options


### Never use std::filesystem functions directly, use Cool::File instead

We wrap the std::filesystem functions and handle exceptions.
Note that using the types are fine (e.g. you can freely use `std::filesystem::path`)

### Never use std::getline

Use Cool::getline instead

std::getline will not remove the \r that is at the end of Windows files, which causes a problem on Linux

// TODO add automatic checks in github action to make sure you didn't use std::getline or a std::filesystem function?

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

## Don't use std::ofstream directly to save a file

Use Cool::File::transactional_save() (or most likely Serialization::save(), AutoSerializer or JsonSerializer)