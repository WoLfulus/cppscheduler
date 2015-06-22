#include <iostream>
#include <scheduler/scheduler.hpp>

using namespace std;

int main(int argc, char* argv[])
{
	scheduler::manager mgr;

	///
	/// Every sunday
	///

	mgr.every().sunday().at("15:00").run([]() {
		cout << "I'll run every sunday at 15:00" << endl; 
	});

	///
	/// Cancel example
	///

	int times = 0;

	mgr.every().seconds().run([&]() 
	{
		cout << "I'll run every second but only 5 times" << endl;
		times++;
		if (times == 5)
		{
			throw scheduler::cancel_task();
		}
	});

	///
	/// Every 30 seconds
	///

	mgr.every(30).seconds().run([]() {
		cout << "I'll run every 30 seconds" << endl;
	});

	///
	/// Every day 
	///

	mgr.every().days().at("12:00").run([]() {
		cout << "I'll run every day at 12:00" << endl;
	});

	///
	/// Runs
	///
	boost::this_thread::sleep_for(boost::chrono::milliseconds(200));

	mgr.start();
	mgr.wait();

	return 0;
}

