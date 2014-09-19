#include <stingray/toolkit/ThreadPool.h>

#include <stingray/log/Logger.h>
#include <stingray/toolkit/bind.h>


namespace stingray
{

	ThreadPool::WorkerWrapper::WorkerWrapper(const std::string& name)
		: _busy(false), _worker(ITaskExecutor::Create(name))
	{ }


	bool ThreadPool::WorkerWrapper::TryAddTask(const function<void()>& task)
	{
		MutexLock l(_mutex);
		if (_busy)
			return false;
		_busy = true;
		_worker->AddTask(bind(&ThreadPool::WorkerWrapper::TaskWrapper, this, task));
		return true;
	}


	bool ThreadPool::WorkerWrapper::TryAddTaskWithTester(const function<void()>& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_mutex);
		if (_busy)
			return false;
		_busy = true;
		_worker->AddTask(bind(&ThreadPool::WorkerWrapper::TaskWrapper, this, task), tester);
		return true;
	}


	void ThreadPool::WorkerWrapper::TaskWrapper(const function<void()>& task)
	{
		STINGRAY_TRY("Couldn't execute task", task());
		{
			MutexLock l(_mutex);
			TOOLKIT_CHECK(_busy, "Internal TaskExecutorPool error!");
			_busy = false;
		}
	}


	///////////////////////////////////////////////////////////////


	ThreadPool::ThreadPool(const std::string& name, u32 maxThreads)
		: _name(name), _maxThreads(maxThreads)
	{ }


	void ThreadPool::Queue(const function<void()>& task)
	{ DoAddTask(bind(&WorkerWrapper::TryAddTask, _1, task)); }


	void ThreadPool::Queue(const function<void()>& task, const FutureExecutionTester& tester)
	{ DoAddTask(bind(&WorkerWrapper::TryAddTaskWithTester, _1, task, tester)); }


	void ThreadPool::DoAddTask(const function<bool(WorkerWrapper*)>& tryAddTaskFunc)
	{
		MutexLock l(_mutex);
		for (Workers::const_iterator it = _workers.begin(); it != _workers.end(); ++it)
		{
			if (tryAddTaskFunc(it->get()))
				return;
		}
		if (_workers.size() < _maxThreads)
		{
			WorkerWrapperPtr w(new WorkerWrapper(StringBuilder() % _name % "_" % _workers.size()));
			_workers.push_back(w);
			TOOLKIT_CHECK(tryAddTaskFunc(w.get()), "Internal TaskExecutorPool error!");
		}
		else
		{
			TOOLKIT_THROW(NotImplementedException());
		}
	}

}
