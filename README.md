# Algorithm analysis homework: Mutex algorithms

Small library that contains multiple locking algorithms implementations:

## Currently present
### TTAS lock
Simple "test and test and set" mutex that optimizes contention compared to simplest possible TAS lock by using cheaper "test" operation on hot path instead of each time using expensive "test and set" operation.

### Reentrant lock
Simplest possible implementation of a reentrant lock holding not one but two atomic variables

### Read-write lock
Simplest read-write lock implementation using read-counter and exclusive-counter.

### MCS lock
Simple fair lock by [John Mellor-Crummey and Michael Scott (1991)](https://www.cs.rochester.edu/u/scott/papers/1991_TOCS_synch.pdf).

### Seqlock
Sequence lock is a read-write lock that utilizes epoch counter and is heavily optimized for reads. Widely [used in Linux kernel](https://docs.kernel.org/locking/seqlock.html).

### Numaamm lock
Experimental NUMA-aware locking algorithm inspired by [RapidStart algorithm](https://github.com/anton-malakhov/oox/blob/main/benchmarks/rapid_start.h) developed by Valery Matskevich under the supervision of [Anton Malakhov](https://scholar.google.com/citations?user=bBSubWgAAAAJ&hl=en).

## Project architecture

Each lock implements a `lockable_t` interface, which provides `exclusive_lock` and `exclusive_unlock` functions.
Read-write locks implement a `read_lockable_t` interfase, which provides `shared_lock` and `shared_unlock` functions.

## Quality assurance

* `ttas_lock_t` provides an exclusive access to a critical session, tests that access critical section from multiple threads ensure that.
* `reentrant_lock_t` provides an exclusive access to a critical session and is reentrant, tests that lock it recursively should ensure its reentrancy.
* `rw_lock_t` states that also multiple readers can access a lock in parallel, tests ensure that.
* `mcs_lock_t` provides complete fairness of the lock.
* `seqlock_t` provides same guarantees as a read-write lock.
* `numaamm_lock_t` guarantees, that in preemption-free environment, lock will be passed to the thread on the same NUMA node.
