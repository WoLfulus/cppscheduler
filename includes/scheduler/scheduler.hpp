//
// The MIT License(MIT)
// 
// Copyright(c) 2015 WoLfulus (wolfulus@gmail.com)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <mutex>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace scheduler
{
	///
	/// Task handler
	///
	typedef std::function<void()> task_handler;

	///
	/// Task cancellation
	///
	class cancel_task : std::exception
	{
	public:
		cancel_task()
			: std::exception("task cancel")
		{
		}
	};

	///
	/// Task
	///
	class task
	{
	private:
		/// Handler
		task_handler handler;

		/// Interval value
		int interval_value;

		/// Interval timespan
		boost::posix_time::time_duration interval;

		/// Next run date
		boost::posix_time::ptime next_run;

		/// Last run date
		boost::posix_time::ptime last_run;

		/// Time of the day to start
		boost::posix_time::time_duration start_time;

		/// Week day to execute
		boost::date_time::weekdays start_day;

		/// Configuration check
		bool is_week_day;

		/// Configuration check
		bool is_day;

	public:
		///
		/// Scheduler access
		///
		friend class manager;

		///
		/// Constructor
		///
		task(int interval = 1) :
			is_day(false),
			is_week_day(false),
			interval_value(interval),
			next_run(boost::posix_time::not_a_date_time),
			last_run(boost::posix_time::not_a_date_time),
			start_time(boost::posix_time::not_a_date_time)
		{
		}

		///
		/// Comparison operator
		///
		bool operator < (const task& other)
		{
			return this->next_run < other.next_run;
		}

		///
		/// Every second
		///
		task& seconds()
		{
			this->interval = boost::posix_time::seconds(this->interval_value);
			return *this;
		}

		///
		/// Every minute
		///
		task& minutes()
		{
			this->interval = boost::posix_time::minutes(this->interval_value);
			return *this;
		}
		
		///
		/// Every hour
		///
		task& hours()
		{
			this->interval = boost::posix_time::hours(this->interval_value);
			return *this;
		}

		///
		/// Every day
		///
		task& days()
		{
			this->is_day = true;
			this->interval = boost::posix_time::hours(24) * this->interval_value;
			return *this;
		}

		///
		/// Every weeks
		///
		task& weeks()
		{
			return this->_weeks(false);
		}

		///
		/// Every sunday
		///
		task& sunday()
		{
			this->start_day = boost::date_time::Sunday;
			return this->_weeks(true);
		}

		///
		/// Every monday
		///
		task& monday()
		{
			this->start_day = boost::date_time::Monday;
			return this->_weeks(true);
		}

		///
		/// Every tuesday
		///
		task& tuesday()
		{
			this->start_day = boost::date_time::Tuesday;
			return this->_weeks(true);
		}

		///
		/// Every wednesday
		///
		task& wednesday()
		{
			this->start_day = boost::date_time::Wednesday;
			return this->_weeks(true);
		}

		///
		/// Every thursday
		///
		task& thursday()
		{
			this->start_day = boost::date_time::Thursday;
			return this->_weeks(true);
		}

		///
		/// Every friday
		///
		task& friday()
		{
			this->start_day = boost::date_time::Friday;
			return this->_weeks(true);
		}

		///
		/// Every saturday
		///
		task& saturday()
		{
			this->start_day = boost::date_time::Saturday;
			return this->_weeks(true);
		}

		///
		/// Specific time
		///
		task& at(std::string value)
		{
			this->start_time = boost::posix_time::time_duration(
					boost::posix_time::duration_from_string(value));
			return *this;
		}

		///
		/// Handler method
		///
		task& run(task_handler handler)
		{
			this->handler = handler;
			this->schedule();
			return *this;
		}

	protected:
		///
		/// Executes the task
		///
		void execute()
		{
			this->last_run = this->next_run;
			this->schedule();
			this->handler();
		}

		//bool flag = false;

		///
		/// Checks if execution can be made
		///
		bool should_execute()
		{
			//if (this->flag)
			//{
			//	this->flag = false;
			//	return true;
			//}
			//else
			//{
			//	this->flag = true;
			//	return false;
			//}
			return this->next_run <= boost::posix_time::microsec_clock::local_time();
		}

		///
		/// Weeks check
		///
		task& _weeks(bool day)
		{
			if (day)
			{
				if (this->interval_value != 1)
				{
					throw std::exception("interval must be 1 when specifying day of week");
				}
				this->is_week_day = true;
			}
			this->interval = boost::posix_time::hours(24) * 7 * this->interval_value;
			return *this;
		}

	private:
		///
		/// Schedules the next run
		///
		void schedule()
		{
			auto now = boost::posix_time::microsec_clock::local_time();
			if (/*this->start_time.is_not_a_date_time() &&*/ !this->last_run.is_not_a_date_time())
			{
				now = this->last_run;
			}

			auto nowdate = boost::gregorian::date(now.date().year(), now.date().month(), now.date().day());

			bool scheduled_day = false;
			int days_ahead = 0;

			this->next_run = now;
			if (this->is_week_day)
			{
				int weekday = this->start_day;
				days_ahead = weekday - this->next_run.date().day_of_week().as_number();
				if (days_ahead < 0)
				{
					days_ahead += 7;
				}
				if (days_ahead == 0)
				{
					scheduled_day = true;
				}
				else
				{
					this->next_run += (boost::posix_time::hours(24) * days_ahead);
				}
			}
			else
			{
				this->next_run += this->interval;
			}

			if (!this->start_time.is_not_a_date_time())
			{
				if (now.time_of_day() >= this->start_time)
				{
					if (scheduled_day)
					{
						this->next_run += this->interval;
					}
				}

				auto next_run_date = boost::gregorian::date(
					this->next_run.date().year(), 
					this->next_run.date().month(), 
					this->next_run.date().day());

				this->next_run = boost::posix_time::ptime(next_run_date);
				this->next_run += this->start_time;

				if (this->is_day)
				{
					if (this->interval_value == 1)
					{
						if (now.time_of_day() < this->start_time)
						{
							this->next_run -= 
								boost::posix_time::time_duration(24, 0, 0, 0);
						}
					}
				}
			}

			std::cout << this->next_run << std::endl;
		}
	};

	///
	/// Scheduler
	///
	class manager
	{
	private:
		///
		/// List of tasks
		///
		std::vector<task*> tasks;

		///
		/// Worker thread
		///
		std::thread worker;

		///
		/// Mutex
		///
		std::recursive_mutex mutex;

		///
		/// Update delay
		///
		boost::chrono::milliseconds delay;

		///
		/// Condition to exit
		///
		bool stopped;

	public:
		///
		/// Constructor
		///
		manager::manager()
			: stopped(false)
		{
		}

		///
		/// Destructor
		///
		manager::~manager()
		{
			this->stop();
		}

		///
		/// Clears the manager
		///
		void clear()
		{
			std::lock_guard<std::recursive_mutex> lock(this->mutex);
			this->tasks.clear();
		}

		///
		/// Creates a task
		///
		task& every(int interval = 1)
		{
			std::lock_guard<std::recursive_mutex> lock(this->mutex);
			task* t = new task(interval);
			this->tasks.push_back(t);
			return *t;
		}

		///
		/// Starts the update thread
		///
		void start(int delay_ms = 50)
		{
			this->delay = boost::chrono::milliseconds(delay_ms);
			this->worker = std::thread(std::bind(&manager::update_loop, this));
		}

		///
		/// Stops the worker
		///
		void stop()
		{
			this->stopped = true;
			if (this->worker.joinable())
			{
				this->worker.join();
			}
		}

		///
		/// Updates the scheduler (manually)
		///
		void update()
		{
			std::lock_guard<std::recursive_mutex> lock(this->mutex);
			auto it = this->tasks.begin();
			while (it != this->tasks.end())
			{
				auto task = *it;
				while (task->should_execute())
				{
					try
					{
						task->execute();
					}
					catch (cancel_task&)
					{
						it = this->tasks.erase(it);
						delete task;
						break;
					}
				}
				it++;
			}
		}

		///
		/// Joins the worker
		///
		void wait()
		{
			if (this->worker.joinable())
			{
				this->worker.join();	
			}
		}

	protected:
		///
		/// Update loop
		///
		void update_loop()
		{
			while (!this->stopped)
			{
				this->update();
				boost::this_thread::sleep_for(this->delay);
			}
		}
	};
}