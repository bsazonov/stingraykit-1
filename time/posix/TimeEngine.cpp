#include <stingray/toolkit/time/posix/TimeEngine.h>

#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/SystemException.h>


namespace stingray {
namespace posix
{

	u64 MonotonicTimer::GetMicroseconds() const
	{
		struct timespec ts = {};
		if (clock_gettime(_clock, &ts) == -1)
			throw SystemException("clock_gettime");
		return (u64)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
	}

	s16 TimeEngine::s_minutesFromUtc = 0;
#ifndef PLATFORM_EMBEDDED
	Mutex	TimeEngine::s_deltaMillisecondsMutex;
	s64		TimeEngine::s_deltaMilliseconds = 0;
#endif

	void TimeEngine::SetMillisecondsSinceEpoch(s64 milliseconds)
	{
#ifdef PLATFORM_EMBEDDED
		struct timeval tv = {};
		tv.tv_sec = milliseconds / 1000;
		tv.tv_usec = (milliseconds % 1000) * 1000;
		if (settimeofday(&tv, 0))
			TOOLKIT_THROW(SystemException("settimeofday"));
#else
		struct timeval tv = {};
		if (gettimeofday(&tv, 0))
			TOOLKIT_THROW(SystemException("gettimeofday"));

		s64 ms = (s64)tv.tv_sec * 1000 + tv.tv_usec / 1000;
		MutexLock l(s_deltaMillisecondsMutex);
		s_deltaMilliseconds = milliseconds - ms;
#endif
	}

	s64 TimeEngine::GetMillisecondsSinceEpoch()
	{
		struct timeval tv = {};
		if (gettimeofday(&tv, 0))
			TOOLKIT_THROW(SystemException("gettimeofday"));

		s64 ms = (s64)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#ifdef PLATFORM_EMBEDDED
		return ms;
#else
		MutexLock l(s_deltaMillisecondsMutex);
		return ms + s_deltaMilliseconds;
#endif
	}

	s16 TimeEngine::GetMinutesFromUtc()
	{ return s_minutesFromUtc; }

	void TimeEngine::SetMinutesFromUtc(s16 minutes)
	{ s_minutesFromUtc = minutes; }

	s64 TimeEngine::MillisecondsFromBrokenDown(const BrokenDownTime& bdTime)
	{
		tm bdt = {};
		bdt.tm_sec = bdTime.Seconds;
		bdt.tm_min = bdTime.Minutes;
		bdt.tm_hour = bdTime.Hours;
		bdt.tm_mday = bdTime.MonthDay;
		bdt.tm_mon = bdTime.Month - 1;
		bdt.tm_year = bdTime.Year - 1900;
		time_t result = mktime(&bdt);
		TOOLKIT_CHECK(result != -1, SystemException("mktime failed while processing bdt = " + bdTime.ToString() + "!"));
		return (s64)result * 1000;
	}

	BrokenDownTime TimeEngine::BrokenDownFromMilliseconds(s64 milliseconds)
	{
		time_t t = milliseconds / 1000;
		tm b = { };

		if (localtime_r(&t, &b) == NULL)
			TOOLKIT_THROW(SystemException("localtime_r failed!"));

		return BrokenDownTime(milliseconds % 1000, b.tm_sec, b.tm_min, b.tm_hour, b.tm_wday, b.tm_mday, b.tm_mon + 1, b.tm_yday, b.tm_year + 1900);
	}

	MonotonicTimer TimeEngine::CreateMonotonicTimer()
	{
#if POSIX_HAVE_MONOTONIC_TIMER
		return MonotonicTimer(CLOCK_MONOTONIC);
#else
		return MonotonicTimer(CLOCK_REALTIME);
#endif
	}

}}