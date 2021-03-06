#include <stingraykit/diagnostics/LibunwindBacktrace.h>

#include <stingraykit/string/Hex.h>
#include <stingraykit/string/string_stream.h>

#include <../libunwind/include/libunwind.h>

namespace stingray
{

	LibunwindBacktrace::LibunwindBacktrace()
		: _size(0)
	{
		unw_cursor_t cursor; unw_context_t uc;
		unw_word_t ip, sp;

		unw_getcontext(&uc);
		unw_init_local(&cursor, &uc);
		while (unw_step(&cursor) > 0 && _size < _backtrace.size())
		{
			unw_get_reg(&cursor, UNW_REG_IP, &ip);
			_backtrace[_size++] = ip;
		}
	}


	std::string LibunwindBacktrace::Get() const
	{
		string_ostream backtrace;
		for (size_t i = 0; i < _size; ++i)
			backtrace << Hex(_backtrace[i], 8).ToString() << " ";

		return backtrace.str();
	}

}
