// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/AsyncProfiler.h>

namespace stingray
{

	static const size_t MaxSessionTime = 60000;

	AsyncProfiler::Session::Session(const AsyncProfilerWeakPtr& profiler, const std::string& name, size_t criticalMs)
		: _profiler(profiler), _threadInfo(Thread::GetCurrentThreadInfo()), _callInfo(new CallInfo(name, null))
	{ Start(criticalMs); }


	AsyncProfiler::Session::Session(const AsyncProfilerWeakPtr& profiler, const NameGetterFunc& nameGetter, size_t criticalMs, const NameGetterTag&)
		: _profiler(profiler), _threadInfo(Thread::GetCurrentThreadInfo()), _callInfo(new CallInfo(null, nameGetter))
	{ Start(criticalMs); }


	void AsyncProfiler::Session::Start(size_t criticalMs)
	{
		AsyncProfilerPtr profiler = _profiler.lock();
		if (!profiler)
		{
			s_logger.Warning() << "Start called on dead profiler";
			return;
		}
		if (_behaviour == Behaviour::Verbose)
			profiler->ReportStart(_callInfo);
		_criticalConnection = profiler->_timer.SetTimeout(criticalMs, bind(&AsyncProfiler::ReportCriticalTime, _callInfo, TimeDuration(criticalMs), _threadInfo));
		_errorConnection = profiler->_timer.SetTimer(MaxSessionTime, bind(&AsyncProfiler::ReportErrorTime, _callInfo, _threadInfo, make_shared<int>(1)));
	}


	AsyncProfiler::Session::~Session()
	{
		AsyncProfilerPtr profiler = _profiler.lock();
		_criticalConnection.Reset();
		_errorConnection.Reset();
		if (!profiler)
		{
			s_logger.Warning() << "profiler session destroyed after profiler death";
			return;
		}
		if (_behaviour == Behaviour::Verbose)
			profiler->ReportEnd(_callInfo, _elapsed.Elapsed());
	}


	STINGRAYKIT_DEFINE_NAMED_LOGGER(AsyncProfiler);

	AsyncProfiler::AsyncProfiler(const std::string& threadName)
		: _timer(threadName, &Timer::DefaultExceptionHandler, false)
	{ }


	void AsyncProfiler::ReportStart(const CallInfoPtr& callInfo)
	{ s_logger.Info() << "Executing " << callInfo->GetName() << "..."; }

	void AsyncProfiler::ReportEnd(const CallInfoPtr& callInfo, TimeDuration time)
	{ s_logger.Info() << callInfo->GetName() << " took " << time; }

	void AsyncProfiler::ReportCriticalTime(const CallInfoPtr& callInfo, TimeDuration criticalTime, const IThreadInfoPtr& threadInfo)
	{
		s_logger.Warning() << callInfo->GetName() << (threadInfo? (" in thread '" + threadInfo->GetName() + "'"): std::string()) << " is being executed for more than " << criticalTime << "! Invoked from:\n" << callInfo->GetBacktrace();
		if (threadInfo)
			threadInfo->RequestBacktrace();
	}

	void AsyncProfiler::ReportErrorTime(const CallInfoPtr& callInfo, const IThreadInfoPtr& threadInfo, const shared_ptr<int>& counter)
	{
		s_logger.Error() << callInfo->GetName() << (threadInfo? (" in thread '" + threadInfo->GetName() + "'"): std::string()) << " is being executed for more than " << TimeDuration((s64)((*counter)++) * (s64)MaxSessionTime) << "! Invoked from:\n" << callInfo->GetBacktrace();
		if (threadInfo)
			threadInfo->RequestBacktrace();
	}

}
