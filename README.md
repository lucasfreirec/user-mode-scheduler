# User-Space Task Scheduler

A cooperative Round-Robin task scheduler implemented in C using `ucontext.h`.

## Features

- Independent task contexts and stacks.
- Cooperative multitasking through `task_yield()`.
- Round-Robin scheduling.
- Task lifecycle management: creation, execution, cancellation, and completion.
- Task state inspection.
- Dynamic memory management for task stacks.

## Build and Run

```bash
make
./main