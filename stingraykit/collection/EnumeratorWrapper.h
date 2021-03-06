#ifndef STINGRAYKIT_COLLECTION_ENUMERATORWRAPPER_H
#define STINGRAYKIT_COLLECTION_ENUMERATORWRAPPER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerator.h>
#include <stingraykit/optional.h>

namespace stingray
{

	template < typename SrcType, typename DestType >
	class EnumeratorWrapper : public IEnumerator<DestType>
	{
		typedef shared_ptr<IEnumerator<SrcType> >					SrcEnumeratorPtr;
		typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;
		typedef function< bool(ConstSrcTypeRef) >					FilterPredicate;
		typedef function< DestType (ConstSrcTypeRef) >				Caster;

	private:
		SrcEnumeratorPtr			_srcEnumerator;
		Caster						_caster;
		FilterPredicate				_filterPredicate;
		mutable optional<SrcType>	_cache;

	public:
		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator) :
			_srcEnumerator(srcEnumerator), _caster(&EnumeratorWrapper::DefaultCast), _filterPredicate(&EnumeratorWrapper::NoFilter)
		{ FindFirst(); }

		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const Caster& caster) :
			_srcEnumerator(srcEnumerator), _caster(caster), _filterPredicate(&EnumeratorWrapper::NoFilter)
		{ FindFirst(); }

		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const Caster& caster, const FilterPredicate& filterPredicate) :
			_srcEnumerator(srcEnumerator), _caster(caster), _filterPredicate(filterPredicate)
		{ FindFirst(); }

		virtual bool Valid() const
		{ return _srcEnumerator->Valid(); }

		virtual DestType Get() const
		{
			if (!_cache)
				return _caster(_srcEnumerator->Get());
			const DestType result = _caster(*_cache);
			_cache.reset();
			return result;
		}

		virtual void Next()
		{
			if (!Valid())
				return;

			do {
				_srcEnumerator->Next();
				if (!Valid())
					break;

				const SrcType& cache = _srcEnumerator->Get();
				if (_filterPredicate(cache))
					_cache.emplace(cache);
			} while (!_cache);
		}

	private:
		void FindFirst()
		{
			while (Valid() && !_cache)
			{
				const SrcType& cache = _srcEnumerator->Get();
				if (_filterPredicate(cache))
					_cache.emplace(cache);
				else
					_srcEnumerator->Next();
			}
		}

		static DestType DefaultCast(ConstSrcTypeRef src)	{ return DestType(src); }
		static bool NoFilter(ConstSrcTypeRef)				{ return true; }
	};


	template<typename SrcType, typename CasterType>
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType> > WrapEnumerator(const shared_ptr<IEnumerator<SrcType> >& src, const CasterType& caster)
	{ return make_shared<EnumeratorWrapper<SrcType, typename function_info<CasterType>::RetType> >(src, caster); }


	template<typename SrcType, typename CasterType, typename FilterPredicate>
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType> > WrapEnumerator(const shared_ptr<IEnumerator<SrcType> >& src, const CasterType& caster, const FilterPredicate& filterPredicate)
	{ return make_shared<EnumeratorWrapper<SrcType, typename function_info<CasterType>::RetType> >(src, caster, filterPredicate); }

}

#endif
