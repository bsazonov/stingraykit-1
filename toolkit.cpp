#include <stingray/toolkit/toolkit.h>

#include <stdio.h>
#include <stdarg.h>

#ifdef __GNUC__ || defined(__clang__)
#	include <cxxabi.h>
#endif

#include <stingray/log/Logger.h>
#include <stingray/toolkit/Backtrace.h>
#include <stingray/toolkit/ScopeExit.h>
#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/array.h>
#include <stingray/toolkit/bind.h>

namespace stingray
{

	NullPtrType null;

	void _append_extended_diagnostics(string_ostream& result, const Detail::IToolkitException& tkit_ex)
	{
		std::string backtrace = tkit_ex.GetBacktrace();
		result << "\n  in function '" << tkit_ex.GetFunctionName() << "'" <<
			"\n  in file '" << tkit_ex.GetFilename() << "' at line " << ToString(tkit_ex.GetLine());

		if (!backtrace.empty())
			result << "\n" << backtrace;
	}

	void DebuggingHelper::BreakpointHere()
	{
	}

	void DebuggingHelper::TerminateWithMessage(const std::string& str)
	{
		std::string backtrace = Backtrace().Get();
		Logger::Error() << "Terminate was requested: " << str << (backtrace.empty() ? "" : ("\nbacktrace: " + backtrace));
		std::terminate();
	}

#ifndef __GNUC__ || defined(__clang__)
	std::string Demangle(const std::string& s)
	{ return s; }
#else
	std::string Demangle(const std::string& s)
	{
		int status = 0;
		char * result = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
		ScopeExitInvoker sei(bind(&free, result));
		return (status != 0) ? s : std::string(result);
	}
#endif

}

#ifdef _STLP_DEBUG_MESSAGE
	void __stl_debug_message(const char * format_str, ...)
	{
		va_list args;
		va_start(args, format_str);

		stingray::array<char, 4096> buffer;
		int count = vsnprintf(buffer.data(), buffer.size(), format_str, args);
		if (count > 0)
		{
			std::string str(buffer.data(), count);
			stingray::Logger::Error() << str << stingray::Backtrace().Get();
		}
		else
			stingray::Logger::Error() << "Can't form stlport error message!\n" << stingray::Backtrace().Get();
		va_end(args);
	}
#endif
