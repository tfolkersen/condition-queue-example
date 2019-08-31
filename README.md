# condition-queue-example
Pthread condition queue example

Pthreads are spawned which alter a shared counter. A mutex and condition(s) are used for synchronization. 

To run example without queue (thread resume order is undefined and some threads should resume out of the order they started waiting in)
```
make a
```

To run example with queue (thread resume order should be FIFO)
```
make b
```

Code is messy and undocumented but I'm assuming nobody will look at this repository.
