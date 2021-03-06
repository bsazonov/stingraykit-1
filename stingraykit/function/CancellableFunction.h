#ifndef STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H
#define STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/PerfectForwarding.h>
#include <stingraykit/TaskLifeToken.h>

namespace stingray
{

	template< typename FunctorType >
	class CancellableFunction : public function_info<FunctorType>
	{
	private:
		FunctorType				_func;
		FutureExecutionTester	_tester;

	public:
		CancellableFunction(const FunctorType& func, const FutureExecutionTester& tester) :
			_func(func), _tester(tester)
		{ }

		STINGRAYKIT_PERFECT_FORWARDING(void, operator (), Do)

		std::string get_name() const
		{ return "{ CancellableFunction: " + get_function_name(_func) + " }"; }

	private:
		template< typename ParamTypeList >
		void Do(const Tuple<ParamTypeList>& params) const
		{
			LocalExecutionGuard guard(_tester);
			if (guard)
				FunctorInvoker::Invoke(_func, params);
		}
	};


	template < typename FuncType >
	CancellableFunction<FuncType> MakeCancellableFunction(const FuncType& func, const FutureExecutionTester& tester)
	{ return CancellableFunction<FuncType>(func, tester); }

}

#endif
