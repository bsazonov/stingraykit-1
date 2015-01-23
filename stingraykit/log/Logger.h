#ifndef STINGRAYKIT_LOG_LOGGER_H
#define STINGRAYKIT_LOG_LOGGER_H


#include <map>
#include <set>
#include <string>

#include <stingraykit/IToken.h>
#include <stingraykit/light_shared_ptr.h>
#include <stingraykit/log/ILoggerSink.h>
#include <stingraykit/log/LogLevel.h>
#include <stingraykit/log/LoggerStream.h>
#include <stingraykit/string/StringUtils.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/atomic.h>
#include <stingraykit/time/ElapsedTime.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

#define DETAIL_DEFINE_NAMED_LOGGER_1(ClassName) stingray::NamedLogger ClassName::s_logger(#ClassName)
#define DETAIL_DEFINE_NAMED_LOGGER_2(ClassName, LoggerName) stingray::NamedLogger ClassName::s_logger(LoggerName)

#define STINGRAYKIT_DEFINE_NAMED_LOGGER(...) STINGRAYKIT_CAT(DETAIL_DEFINE_NAMED_LOGGER_, STINGRAYKIT_NARGS(__VA_ARGS__))(__VA_ARGS__)


#define STINGRAYKIT_TRY(ErrorMessage_, ...) \
		do { \
			DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) { STINGRAYKIT_STATIC_LOGGER.Warning() << (ErrorMessage_) << ":\n" << stingray::diagnostic_information(ex); } \
		} while (0)


#define STINGRAYKIT_TRY_EX(LogLevel_, ErrorMessage_, ...) \
		do { \
			DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) { STINGRAYKIT_STATIC_LOGGER.LogLevel_() << (ErrorMessage_) << ":\n" << stingray::diagnostic_information(ex); } \
		} while (0)


#define STINGRAYKIT_TRY_NO_MESSAGE(...) STINGRAYKIT_TRY(#__VA_ARGS__, __VA_ARGS__)


	class Logger
	{
	public:
		static void SetLogLevel(LogLevel logLevel);
		static LogLevel GetLogLevel();

		static void SetLogLevel(const std::string& loggerName, LogLevel logLevel);
		static LogLevel GetLogLevel(const std::string& loggerName);

		static void SetBacktraceEnabled(const std::string& loggerName, bool enable);
		static bool GetBacktraceEnabled(const std::string& loggerName);

		static void GetLoggerNames(std::set<std::string>& out);

		static LoggerStream Stream(LogLevel logLevel, DuplicatingLogsFilter* duplicatingLogsFilter = NULL);

		static LoggerStream Trace()	{ return Stream(LogLevel::Trace); }
		static LoggerStream Debug()	{ return Stream(LogLevel::Debug); }
		static LoggerStream Info()		{ return Stream(LogLevel::Info); }
		static LoggerStream Warning()	{ return Stream(LogLevel::Warning); }
		static LoggerStream Error()	{ return Stream(LogLevel::Error); }

		static Token AddSink(const ILoggerSinkPtr& sink);

		static void Log(LogLevel logLevel, const std::string& message);
		static void Log(const std::string& loggerName, LogLevel logLevel, const std::string& message);
	};


	class NamedLogger
	{
		struct OptionalLogLevel
		{
			STINGRAYKIT_ENUM_VALUES(
				Trace	= LogLevel::Trace,
				Debug	= LogLevel::Debug,
				Info	= LogLevel::Info,
				Warning	= LogLevel::Warning,
				Error	= LogLevel::Error,
				Null);

			static OptionalLogLevel FromLogLevel(optional<LogLevel> level)
			{ return level ? (OptionalLogLevel::Enum)level->val() : OptionalLogLevel::Null; }

			static optional<LogLevel> ToLogLevel(OptionalLogLevel level)
			{
				if (level == Null)
					return null;
				return LogLevel((LogLevel::Enum)level.val());
			}

			STINGRAYKIT_DECLARE_ENUM_CLASS(OptionalLogLevel);
		};

	private:
		const char*						_name;
		mutable DuplicatingLogsFilter	_duplicatingLogsFilter;
		atomic<OptionalLogLevel>		_logLevel;
		atomic<bool>					_backtrace;

	public:
		NamedLogger(const char* name);
		~NamedLogger();

		LogLevel GetLogLevel() const
		{
			optional<LogLevel> logLevel = OptionalLogLevel::ToLogLevel(_logLevel.load(memory_order_relaxed));
			return logLevel ? *logLevel : Logger::GetLogLevel();
		}

		void SetLogLevel(LogLevel logLevel)
		{ _logLevel.store(OptionalLogLevel::FromLogLevel(logLevel), memory_order_relaxed); }

		inline bool BacktraceEnabled() const
		{ return _backtrace; }

		inline void EnableBacktrace(bool enable)
		{ _backtrace = enable; }

		LoggerStream Stream(LogLevel logLevel) const;

		LoggerStream Trace()	const { return Stream(LogLevel::Trace); }
		LoggerStream Debug()	const { return Stream(LogLevel::Debug); }
		LoggerStream Info()		const { return Stream(LogLevel::Info); }
		LoggerStream Warning()	const { return Stream(LogLevel::Warning); }
		LoggerStream Error()	const { return Stream(LogLevel::Error); }

		void Log(LogLevel logLevel, const std::string& message)
		{ Stream(logLevel) << message; }
	};


	namespace Detail
	{
		struct NamedLoggerAccessor
		{
		private:
			NamedLogger* _logger;

		public:
			NamedLoggerAccessor() : _logger(null)
			{ }

			NamedLoggerAccessor(NamedLogger& logger) : _logger(&logger)
			{ }

			NamedLoggerAccessor& operator =(NullPtrType& n)
			{ return *this; }

			NamedLoggerAccessor& operator =(NamedLogger& logger)
			{ _logger = &logger; return *this; }

			LogLevel GetLogLevel() const		{ return _logger ? _logger->GetLogLevel() : Logger::GetLogLevel(); }

			LoggerStream Stream(LogLevel logLevel)	{ return _logger ? _logger->Stream(logLevel) : Logger::Stream(logLevel); }

			LoggerStream Trace()					{ return _logger ? _logger->Trace() : Logger::Trace(); }
			LoggerStream Debug()					{ return _logger ? _logger->Debug() : Logger::Debug(); }
			LoggerStream Info()						{ return _logger ? _logger->Info() : Logger::Info(); }
			LoggerStream Warning()					{ return _logger ? _logger->Warning() : Logger::Warning(); }
			LoggerStream Error()					{ return _logger ? _logger->Error() : Logger::Error(); }
		};
	}


	template < typename T >
	class HexFormatter
	{
		const T&	_val;
		size_t		_width;

	public:
		inline explicit HexFormatter(const T& val, size_t width)
			: _val(val), _width(width)
		{ }

		std::string ToString() const
		{
			typedef typename IntType<sizeof(T) * 8, false>::ValueT CastTo;
			CompileTimeAssert<sizeof(CastTo) >= sizeof(T)> ERROR__T_is_bigger_than_CastTo;
			CompileTimeAssert<sizeof(u64) >= sizeof(T)> ERROR__T_is_bigger_than_u64;
			(void)ERROR__T_is_bigger_than_CastTo;
			(void)ERROR__T_is_bigger_than_u64;

			string_ostream ss;
			ToHexImpl(ss, (u64)(CastTo)_val, _width);
			return ss.str();
		}
	};


	template < typename T >
	inline HexFormatter<T> Hex(const T& val, size_t width = 0)
	{ return HexFormatter<T>(val, width); }


	class ActionLogger
	{
	private:
		NamedLogger*	_namedLogger;
		std::string		_action;
		ElapsedTime		_elapsedTime;

	public:
		ActionLogger(const std::string& action);
		ActionLogger(NamedLogger& namedLogger, const std::string& action);
		~ActionLogger();
	};


	void LogException(const std::exception& ex);
	void LogExceptionTo(NamedLogger& namedLogger, const std::exception& ex);

	class ExceptionLogger
	{
	private:
		NamedLogger*	_logger;

	public:
		ExceptionLogger(NamedLogger& logger) : _logger(&logger) { }
		void operator () (const std::exception& ex) const { LogExceptionTo(*_logger, ex); }
	};


	namespace Detail {
	namespace LoggerDetail
	{
		extern NullPtrType	s_logger; // Static logger fallback

		class LogExceptionsGuard
		{
		private:
			const char* _text;

		public:
			LogExceptionsGuard(const char* text) : _text(text) { }
			~LogExceptionsGuard();

			operator bool() const { return true; }
		};
	}}

#define LOG_EXCEPTIONS(...) (Detail::LoggerDetail::LogExceptionsGuard(#__VA_ARGS__) ? (__VA_ARGS__) : (__VA_ARGS__))

#define DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR \
	::stingray::Detail::NamedLoggerAccessor STINGRAYKIT_CAT(detail_static_logger_accessor, __LINE__); \
	do { \
		using namespace ::stingray::Detail::LoggerDetail; \
		STINGRAYKIT_CAT(detail_static_logger_accessor, __LINE__) = s_logger; \
	} while (0)

#define STINGRAYKIT_STATIC_LOGGER \
	(STINGRAYKIT_CAT(detail_static_logger_accessor, __LINE__))

	class Tracer
	{
	private:
		Detail::NamedLoggerAccessor	_logger;
		const char*					_funcName;
		ElapsedTime					_elapsedTime;

	public:
		Tracer(const Detail::NamedLoggerAccessor& logger, const char* funcName);
		~Tracer();
	};


#if !defined(PRODUCTION_BUILD)
#	define TRACER	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; stingray::Tracer tracer(STINGRAYKIT_STATIC_LOGGER, STINGRAYKIT_FUNCTION)
#else
#	define TRACER
#endif

	/** @} */

}


#endif