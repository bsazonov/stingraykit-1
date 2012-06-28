#if !HAVE_TASK_EXECUTOR

#include <stingray/toolkit/ThreadTaskExecutor.h>
#include <stingray/toolkit/bind.h>


namespace stingray
{


		ThreadTaskExecutor::ThreadTaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler)
			: _working(true), _paused(false), _exceptionHandler(exceptionHandler)
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


		void ThreadTaskExecutor::AddTask(const TaskType& task, const task_alive_token& token)
		{
			MutexLock l(_syncRoot);
			_queue.push(std::make_pair(task, token.get_value()));
			_condVar.Broadcast();
		}

		void ThreadTaskExecutor::AddTask(const TaskType& task)
		{ AddTask(task, _token); }


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
						InvokeTask(top.first, top.second);
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
