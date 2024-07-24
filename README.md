# Coal

<div align="center">

[![CI](https://github.com/3uclid3/coal/actions/workflows/ci.yml/badge.svg)](https://github.com/3uclid3/coal/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/3uclid3/coal/graph/badge.svg?token=WoAKEzkrWy)](https://codecov.io/gh/3uclid3/coal)

</div>

## Overview

Coal is a simple header-only C++ library that provides flexible and composable memory allocators. It draws inspiration from Andrei Alexandrescu's presentation at CppCon 2015, offering a modular approach to memory management.

## Usage

Here's a basic example of how to use the composable allocators:

```cpp
// includes

using corruption_detector = coal::memory_corruption_detector<std::uint32_t, 0xDEADDEAD>;

constexpr std::size_t size(std::size_t s)
{
    return sizeof(corruption_detector) + s;
}

using allocator_t = coal::segregator_allocator<
        coal::slab_allocator<coal::malloc_allocator, 2048, 8, 16, 32, 64, 128, 512, 1024>,
        coal::free_list_allocator<coal::prefixed_size_allocator<coal::malloc_allocator>, coal::free_list_strategy::limited_size<coal::free_list_strategy::best_fit, 64>>,
        1024>>;

allocator_t allocator;

coal::memory_block blk = allocator.allocate(24);

std::cout << "Allocated ptr=" << blk.buffer << " of size " << blk.size;

allocator.deallocate(blk);
```

## Requirements

To use Coal, the following dependencies are required:

- Hedley: A C/C++ header-only library for feature detection and portable compiler attributes.

Include Hedley in your project to ensure compatibility with various compilers and platforms. You can find Hedley on its [GitHub repository](https://github.com/nemequ/hedley).

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Acknowledgements

Special thanks to Andrei Alexandrescu for his insightful talk at CppCon 2015, which inspired this project.

## Resources

- [CppCon 2015 Talk by Andrei Alexandrescu](https://www.youtube.com/watch?v=LIb3L4vKZ7U)
