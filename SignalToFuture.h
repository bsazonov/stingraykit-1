#ifndef STINGRAY_TOOLKIT_SIGNALTOFUTURE_H
#define STINGRAY_TOOLKIT_SIGNALTOFUTURE_H

#include <stingray/toolkit/future.h>
#include <stingray/toolkit/signals.h>

namespace stingray {

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	template<typename ParamType>
	class SignalToFutureWrapper
	{
		signal_connection_holder		_connection;
		promise<ParamType>				_promise;
		shared_future<ParamType>		_future;

	public:
		template<typename SignalSignature>
		explicit SignalToFutureWrapper(const signal<SignalSignature>& s)
		{ _connection = s.connect(bind(&SignalToFutureWrapper::operator(), this, _1)); }

		template<typename SignalSignature>
		explicit SignalToFutureWrapper(const signal_connector<SignalSignature>& s)
		{ _connection = s.connect(bind(&SignalToFutureWrapper::operator(), this, _1)); }

		~SignalToFutureWrapper()
		{ _connection.disconnect(); }

		shared_future<ParamType> GetFuture()
		{
			if(!_future.valid())
				_future = _promise.get_future().share();
			return _future;
		}

		void operator()(const ParamType& p)
		{ _promise.set_value(p); }
	};


	template<>
	class SignalToFutureWrapper<void>
	{
		signal_connection_holder	_connection;
		promise<void>				_promise;
		shared_future<void>			_future;

	public:
		template<typename SignalSignature>
		explicit SignalToFutureWrapper(const signal<SignalSignature>& s)
		{ _connection = s.connect(bind(&SignalToFutureWrapper::operator(), this)); }

		~SignalToFutureWrapper()
		{ _connection.disconnect(); }

		shared_future<void> GetFuture()
		{
			if(!_future.valid())
				_future = _promise.get_future().share();
			return _future;
		}

		void operator()()
		{ _promise.set_value(); }
	};

	/** @} */

}


#endif

