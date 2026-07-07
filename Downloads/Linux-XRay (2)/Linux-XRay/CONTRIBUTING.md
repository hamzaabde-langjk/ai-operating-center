# Contributing to Linux X-Ray Vision

## Code Standards

- C++20 features encouraged
- No raw new/delete - use smart pointers
- No global variables
- RAII for all resources
- Doxygen comments for all public APIs
- clang-format and clang-tidy compliance

## Testing

- Unit tests with GoogleTest
- Integration tests for eBPF programs
- Performance benchmarks
- Memory leak detection with Valgrind/ASan

## Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Write tests for new functionality
4. Ensure all tests pass
5. Update documentation
6. Submit PR with clear description

## Commit Messages

Follow conventional commits:
- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation
- `test:` Tests
- `refactor:` Code refactoring
- `perf:` Performance improvement
- `security:` Security fix
