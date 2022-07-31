# Producer-Consumer on a concurrent Hash Table

* The client(producer) proccess generates a command buffer of hash table operations (FIND, INSERT, REMOVE). This buffer is sent to a server(consumer) process using the POSIX shared-memory API. The client indefinately repeats this process.

* The server process receives the command buffer, splits the command workload to threads and dispatches them. The threads concurrently access the hash table by executing hash table operations (FIND, INSERT, REMOVE). The server indefinately repeats this process.

* Producer-Consumer synchronization is handled by named semaphores shared among processes.

## Regarding the hash table implementation
* A compile time allocated array is used to store the bucket lists.
* Each bucket is characterized by a unique key
* Each bucket list is indexed by a hash number. A simple modulo hash function is used.
* Read/Write access to each bucket list is protected by it's corresponding reader/writer mutex. A compile time allocated array is used to store the mutexes.
* FIND (read) operations access a bucket list using a reader lock.
* INSERT/REMOVE (write) operations access a bucket list a writer lock.
* The process of hashing and picking the right bucket list does not need to be protected by a mutex. Concurrent INSERT/REMOVE operations that operate on different bucket lists are mutually exclusive.

## Run instructions
* Make sure `run.sh` script has execution permisions.
* Run `./run.sh`.
* In the script you can configure the number of server threads as well as the total number of commands generated by the client.

## Testing
Server-side tests have been written to validate the correct operation of the hash table. To run these tests
* Uncomment the test functions ath the begining of `main` in `server.cpp`. Configure the commands and thread number in the test bodies.
* Produce the server executable using the `make` command.
* Run `./server`.

## Future work
* Dynamicaly resizable hash table can be supported by using two more global mutexes. One is used to access the hash table, the other to access a map of mutexes.
* INSERT/REMOVE operation synchronization can be fine-grained further by using hand-over-hand locking operated on queue locks or by using a lazy syncronization mechanism.
* Cache locality can be optimized by aligning cache lines.