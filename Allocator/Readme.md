# Concurrent Memory Allocator

* The memory allocator uses the `sbrk()` system call to reserve more memory on the heap.
* Each memory block containts as header information the size of allocated memory in bytes and a pointer to the start of the allocated memory
* A Free List data structure is used to store the dealocated blocks. The Free List is parsed on each allocation for the possibility of re-using deallocated memory blocks.
* First-Fit algorithm is used to recycle deallocated memory blocks. Best-Fit and Worst-Fit are also implemented. The algorithm can be changed by uncomenting the body of `FindBlock()` in `alloc.cpp`.

## Regarding concurrency
* System calls to `brk()` and `sbrk()` are protected by a global mutex.
* Access to the Free List is protected by a reader/writer mutex.
* Read access of the FreeList on allocations is done using a reader lock.
* Inserting/Removing elements from the FreeList is done using a writer lock.

## Run instructions
* Make sure `run.sh` script has execution permisions.
* Run `./run.sh`.
* In the script you can configure the number of threads used in tests.

## Testing
Tests have been written to validate the allocator. To run these tests
* Uncomment the test functions ath the begining of `main` in `main.cpp`. Configure the parameters in the test bodies.
* Run `./run.sh`.

## Future work
* More allocation algorithms can be implemented like Next-Fit.
* Instead of thread contention to a global FreeList, a pool of FreeLists can be used. A thread can have access to a unique FreeList at a time, thus reducing mutex contention.
* FreeLists that contain blocks of a specific size can also be used to adhere to the requirements of a specific allocation size.
* Memory blocks can be aligned to optimize cache locality.
