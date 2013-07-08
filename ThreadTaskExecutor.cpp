#if !HAVE_TASK_EXECUTOR

#include <stingray/log/Logger.h>
#include <stingray/toolkit/ThreadTaskExecutor.h>
#include <stingray/toolkit/bind.h>


namespace stingray
{

		static const size_t TaskCountLimit = 1024;

		ThreadTaskExecutor::ThreadTaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler)
			: _name(name), _working(true), _paused(false), _exceptionHandler(exceptionHandler)
		{ _worker.reset(new Thread(name, bind(&ThreadTaskExecutor::ThreadFunc, this))); }


		ThreadTaskExecutor::~ThreadTaskExecutor()
		{
			//_worker->Interrupt();
			{
				MutexLock l(_syncRoot);
				_working = false;
				_condVar.Broadcast();
			}
			_worker->Join();
		}


		void ThreadTaskExecutor::AddTask(const TaskType& task, const FutureExecutionTester& tester)
		{
			MutexLock l(_syncRoot);
			_queue.push(std::make_pair(task, tester));
			if (_queue.size() > TaskCountLimit)
				Logger::Error() << "[ThreadTaskExecutor] Task queue size limit exceeded for executor '" << _name << "': " << _queue.size();
			_condVar.Broadcast();
		}

		void ThreadTaskExecutor::AddTask(const TaskType& task)
		{ AddTask(task, null); }


		void ThreadTaskExecutor::Pause(bool pause)
		{
			MutexLock l(_syncRoot);
			if (_paused == pause)
				return;

			_paused = pause;
			if (!_paused)
				_condVar.Broadcast();
		}


		void ThreadTaskExecutor::ThreadFunc()
		{
			MutexLock l(_syncRoot);
			while (_working)
			{
				while (!_paused && _working && !_queue.empty())
				{
					TaskPair top = _queue.front();
					_queue.pop();
					try
					{
						MutexUnlock ul(l);
						LocalExecutionGuard guard;
						if (top.second.Execute(guard))
							top.first();
						Thread::InterruptionPoint();
					}
					catch(const std::exception& ex)
					{ _exceptionHandler(ex); }
				}
				if (_working)
					_condVar.Wait(_syncRoot);
			}
		}

}

#endif
