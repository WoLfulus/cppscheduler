cppscheduler
============

Simple C++11 task scheduler based on dbader's [schedule](https://github.com/dbader/schedule "schedule") library (python).

In-process scheduler for periodic jobs that uses builder pattern for configuration. cppscheduler lets you run any thing convertible to a **std::function<void()>** periodically at pre-determined intervals using a simple human-friendly syntax.

Features
--------

- Simple fluent/builder API
- Allows single or multi thread updates
- Thread-safe
- Single-file header only

Dependencies
------------

**cppscheduler** depends on **boost**. If you want to compile the Visual Studio 2013 project, you must set BOOST_DIR environment variable with the path do boost include directory.

Usage
-----

Creating the scheduler manager.

```c++

    scheduler::manager mgr;
```

Registering tasks

```cpp
    
    mgr.every(10).minutes().run([]() {
        cout << "Hello world every 10 minutes." << endl;
    });
    
    mgr.every().hours().run([]() {
        cout << "Hello world every hour." << endl;
    });
    
    mgr.every().days().run([]() {
        cout << "Hello world every day." << endl;
    });
            
    mgr.every().days().at("09:00").run([]() {
        cout << "Hello world every day at 09:00am." << endl;
    });

    mgr.every(2).days().at("15:00").run([]() {
        cout << "Hello world every 2 days at 15:00pm." << endl;
    });

    mgr.every().sunday().at("16:00").run([]() {
        cout << "Hello world every sunday at 16:00pm." << endl;
    });

```

Canceling a task

```cpp
    
    mgr.every().seconds().run([]() {
        cout << "Hello once." << endl;
        throw scheduler::cancel_task();
    });

```

Updating the scheduler on your own

```cpp

    while (true)
	{
		mgr.update();
		boost::this_thread::sleep_for(
			boost::chrono::milliseconds(200));
	}

```

Leaving updates to the scheduler 

*Note that this will create an additional thread and all updates will run under scheduler's thread*


```cpp
 
	// 200 is optional (the delay for each update)
	mgr.start(200);
	
```

Distributed under the MIT license. See ``LICENSE`` for more information.