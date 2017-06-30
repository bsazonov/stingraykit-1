// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/timer/Timer.h>

#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/function/CancellableFunction.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/function_name_getter.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/FunctionToken.h>
#include <stingraykit/TaskLifeToken.h>

#include <list>
#include <map>

namespace stingray
{

	class Timer::CallbackQueue
	{
		typedef std::list<CallbackInfoPtr>					ContainerInternal;
		typedef std::map<TimeDuration, ContainerInternal>	Container;

	private:
		Mutex			_mutex;
		Container		_container;

	public:
		typedef ContainerInternal::iterator iterator;

		inline Mutex& Sync()
		{ return _mutex; }

		inline bool IsEmpty() const
		{
			MutexLock l(_mutex);
			return _container.empty();
		}

		CallbackInfoPtr Top() const;
		void Push(const CallbackInfoPtr& ci);
		void Erase(const CallbackInfoPtr& ci);
		CallbackInfoPtr Pop();
	};

	class Timer::CallbackInfo
	{
		STINGRAYKIT_NONCOPYABLE(CallbackInfo);

		typedef function<void()>			FuncT;
		typedef CallbackQueue::iterator		QueueIterator;

	private:
		FuncT						_func;
		TimeDuration				_timeToTrigger;
		optional<TimeDuration>		_period;
		TaskLifeToken				_token;
		optional<QueueIterator>		_iterator;

	private:
		friend class CallbackQueue;

		void SetIterator(const optional<QueueIterator>& it)		{ _iterator = it; }
		const optional<QueueIterator>& GetIterator() const		{ return _iterator; }

	public:
		CallbackInfo(const FuncT& func, const TimeDuration& timeToTrigger, const optional<TimeDuration>& period, const TaskLifeToken& token)
			:	_func(func),
				_timeToTrigger(timeToTrigger),
				_period(period),
				_token(token)
		{ }

		const FuncT& GetFunc() const							{ return _func; }
		FutureExecutionTester GetExecutionTester() const 		{ return _token.GetExecutionTester(); }
		void Release()											{ _token.Release(); }

		bool IsPeriodic() const									{ return _period.is_initialized(); }
		void Restart(const TimeDuration& currentTime)
		{
			STINGRAYKIT_CHECK(_period, "CallbackInfo::Restart internal error: _period is set!");
			_timeToTrigger = currentTime + *_period;
		}
		TimeDuration GetTimeToTrigger() const					{ return _timeToTrigger; }
	};

	Timer::CallbackInfoPtr Timer::CallbackQueue::Top() const
	{
		MutexLock l(_mutex);
		if (!_container.empty())
		{
			const ContainerInternal& listForTop = _container.begin()->second;
			STINGRAYKIT_CHECK(!listForTop.empty(), "try to get callback from empty list");
			return listForTop.front();
		}
		else
			return null;
	}

	void Timer::CallbackQueue::Push(const CallbackInfoPtr& ci)
	{
		MutexLock l(_mutex);
		ContainerInternal& listToInsert = _container[ci->GetTimeToTrigger()];
		ci->SetIterator(listToInsert.insert(listToInsert.end(), ci));
	}

	void Timer::CallbackQueue::Erase(const CallbackInfoPtr& ci)
	{
		MutexLock l(_mutex);
		const optional<iterator>& it = ci->GetIterator();
		if (!it)
			return;

		TimeDuration keyToErase = ci->GetTimeToTrigger();
		ContainerInternal& listToErase = _container[keyToErase];
		listToErase.erase(*it);
		if (listToErase.empty())
			_container.erase(keyToErase);
		ci->SetIterator(null);
	}

	Timer::CallbackInfoPtr Timer::CallbackQueue::Pop()
	{
		MutexLock l(_mutex);
		STINGRAYKIT_CHECK(!_container.empty(), "popping callback from empty map");
		ContainerInternal& listToPop = _container.begin()->second;
		STINGRAYKIT_CHECK(!listToPop.empty(), "popping callback from empty list");

		CallbackInfoPtr ci = listToPop.front();
		listToPop.pop_front();
		if (listToPop.empty())
			_container.erase(ci->GetTimeToTrigger());
		ci->SetIterator(null);
		return ci;
	}


	STINGRAYKIT_DEFINE_NAMED_LOGGER(Timer);

	Timer::Timer(const std::string& timerName, const ExceptionHandler& exceptionHandler, bool profileCalls)
		:	_timerName(timerName),
			_exceptionHandler(exceptionHandler),
			_profileCalls(profileCalls),
			_alive(true),
			_queue(make_shared<CallbackQueue>()),
			_worker(make_shared<Thread>(timerName, bind(&Timer::ThreadFunc, this, not_using(_1))))
	{ }


	Timer::~Timer()
	{
		{
			MutexLock l(_queue->Sync());
			_alive = false;
			_cond.Broadcast();
		}
		_worker.reset();

		MutexLock l(_queue->Sync());
		while(!_queue->IsEmpty())
		{
			CallbackInfoPtr top = _queue->Pop();

			MutexUnlock ul(l);
			LocalExecutionGuard guard(top->GetExecutionTester());

			top.reset();
			if (guard)
			{
				s_logger.Warning() << "killing timer " << _timerName << " which still has some functions to execute";
				break;
			}
		}
	}


	Token Timer::SetTimeout(const TimeDuration& timeout, const function<void()>& func)
	{
		MutexLock l(_queue->Sync());

		CallbackInfoPtr ci = make_shared<CallbackInfo>(func, _monotonic.Elapsed() + timeout, null, TaskLifeToken());
		_queue->Push(ci);
		_cond.Broadcast();

		return MakeToken<FunctionToken>(bind(&Timer::RemoveTask, _queue, ci));
	}


	Token Timer::SetTimer(const TimeDuration& interval, const function<void()>& func)
	{ return SetTimer(interval, interval, func); }


	Token Timer::SetTimer(const TimeDuration& timeout, const TimeDuration& interval, const function<void()>& func)
	{
		MutexLock l(_queue->Sync());

		CallbackInfoPtr ci = make_shared<CallbackInfo>(func, _monotonic.Elapsed() + timeout, interval, TaskLifeToken());
		_queue->Push(ci);
		_cond.Broadcast();

		return MakeToken<FunctionToken>(bind(&Timer::RemoveTask, _queue, ci));
	}


	void Timer::AddTask(const function<void()>& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_queue->Sync());

		CallbackInfoPtr ci = make_shared<CallbackInfo>(MakeCancellableFunction(task, tester), _monotonic.Elapsed(), null, TaskLifeToken::CreateDummyTaskToken());
		_queue->Push(ci);
		_cond.Broadcast();
	}


	void Timer::RemoveTask(const CallbackQueuePtr& queue, const CallbackInfoPtr& ci)
	{
		{
			MutexLock l(queue->Sync());
			queue->Erase(ci);
		}
		ci->Release();
	}


	std::string Timer::GetProfilerMessage(const function<void()>& func)
	{ return StringBuilder() % get_function_name(func) % " in Timer '" % _timerName % "'"; }


	void Timer::ThreadFunc()
	{
		MutexLock l(_queue->Sync());

		while (_alive)
		{
			if (_queue->IsEmpty())
			{
				_cond.Wait(_queue->Sync());
				continue;
			}

			CallbackInfoPtr top = _queue->Top();
			if (top->GetTimeToTrigger() <= _monotonic.Elapsed())
			{
				_queue->Pop();

				{
					MutexUnlock ul(l);
					LocalExecutionGuard guard(top->GetExecutionTester());
					if (!guard)
					{
						top.reset();
						continue;
					}

					if (top->IsPeriodic())
						top->Restart(_monotonic.Elapsed());

					try
					{
						if (_profileCalls)
						{
							AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&Timer::GetProfilerMessage, this, ref(top->GetFunc())), 10000, AsyncProfiler::Session::NameGetterTag());
							(top->GetFunc())();
						}
						else
							(top->GetFunc())();
					}
					catch(const std::exception &ex)
					{ _exceptionHandler(ex); }

					if (!top->IsPeriodic())
						top.reset();
				}

				if (top)
					_queue->Push(top);
			}
			else //top timer not triggered
			{
				const TimeDuration waitTime = top->GetTimeToTrigger() - _monotonic.Elapsed();
				top.reset();
				if (waitTime > TimeDuration())
					_cond.TimedWait(_queue->Sync(), waitTime);
			}
		}

		const TimeDuration currentTime = _monotonic.Elapsed();
		while (!_queue->IsEmpty())
		{
			CallbackInfoPtr top = _queue->Pop();

			if (top->GetTimeToTrigger() <= currentTime)
				break;

			MutexUnlock ul(l);
			LocalExecutionGuard guard(top->GetExecutionTester());
			if (!guard)
			{
				top.reset();
				continue;
			}

			try
			{
				if (_profileCalls)
				{
					AsyncProfiler::Session profiler_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&Timer::GetProfilerMessage, this, ref(top->GetFunc())), 10000, AsyncProfiler::Session::NameGetterTag());
					(top->GetFunc())();
				}
				else
					(top->GetFunc())();
			}
			catch(const std::exception &ex)
			{ _exceptionHandler(ex); }

			top.reset();
		}
	}


	void Timer::DefaultExceptionHandler(const std::exception& ex)
	{ s_logger.Error() << "Timer func exception: " << ex; }

}
