# CA - Composable Allocator

<div align="center">

[![CI](https://github.com/3uclid3/composable-allocator/actions/workflows/ci.yml/badge.svg)](https://github.com/3uclid3/composable-allocator/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/3uclid3/composable-allocator/graph/badge.svg?token=WoAKEzkrWy)](https://codecov.io/gh/3uclid3/composable-allocator)

</div>

## Overview

The Composable Allocator (CA) is a simple header-only C++ library that provides flexible and composable memory allocators. It draws inspiration from Andrei Alexandrescu's presentation at CppCon 2015, offering a modular approach to memory management.

## Usage

Here's a basic example of how to use the composable allocators:

```cpp
// includes

using corruption_detector = ca::memory_corruption_detector<u32_t, 0xDEADDEAD>;

constexpr size_t size(size_t s)
{
    return sizeof(corruption_detector) + s;
}

using allocator_t = ca::segregator_allocator<
        ca::slab_allocator<ca::malloc_allocator, 2048, 8, 16, 32, 64, 128, 512, 1024>,
        ca::free_list_allocator<ca::prefixed_size_allocator<ca::malloc_allocator>, ca::limited_size_free_list_strategy<ca::best_fit_free_list_strategy, 64>>,
        1024>>;

allocator_t allocator;

ca::memory_block blk = allocator.allocate(24);

std::cout << "Allocated ptr=" << blk.buffer << " of size " << blk.size;

allocator.deallocate(blk);
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Acknowledgements

Special thanks to Andrei Alexandrescu for his insightful talk at CppCon 2015, which inspired this project.

## Resources

- [CppCon 2015 Talk by Andrei Alexandrescu](https://www.youtube.com/watch?v=LIb3L4vKZ7U)
