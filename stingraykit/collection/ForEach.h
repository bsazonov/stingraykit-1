#ifndef STINGRAYKIT_COLLECTION_FOREACH_H
#define STINGRAYKIT_COLLECTION_FOREACH_H

#include <stingraykit/collection/EnumeratorWrapper.h>
#include <stingraykit/collection/IEnumerator.h>
#include <stingraykit/dynamic_caster.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 *
	 * @brief A foreach loop
	 * @param[in] ... &lt;type&gt; &lt;name&gt; IN &lt;enumerable or enumerator&gt; [ WHERE &lt;some condition&gt; ]
	 * @par Example:
	 * @code
	 * FOR_EACH(ISomeObjectPtr obj IN something->GetObjectCollection())
	 * {
	 *     ISomeOtherObjectPtr other_obj = dynamic_pointer_cast<ISomeOtherObject>(obj);
	 *     if (!other_obj)
	 *         continue;
	 *     if (other_obj->GetProperty() == 1)
	 *         continue;
	 *     other_obj->SomeMethod();
	 * }
	 * // or
	 * FOR_EACH(ISomeOtherObjectPtr other_obj IN something->GetObjectCollection() WHERE other_obj->GetProperty() != 1)
	 *     other_obj->SomeMethod();
	 * @endcode
	 */
#define FOR_EACH(...) FOR_EACH__IMPL(__VA_ARGS__)


	namespace Detail
	{
		template < typename Signature >
		struct GetItemTypeFromItemDecl;

		template < typename T >
		struct GetItemTypeFromItemDecl< void (*)(T) >
		{ typedef T	ValueT; };


		template < typename SrcType, typename LifeAssurance >
		class EnumeratorCaster
		{
			typedef shared_ptr<IEnumerator<SrcType> >					SrcEnumeratorPtr;
			typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;


			class Proxy : public EnumeratorWrapper<SrcType, SrcType>
			{
				typedef EnumeratorWrapper<SrcType, SrcType>	base;

			private:
				shared_ptr<LifeAssurance>	_assurance;

			public:
				Proxy(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<LifeAssurance>& assurance)
					: base(srcEnumerator), _assurance(assurance)
				{ }
			};

			template < typename DestType >
			class CastProxy : public EnumeratorWrapper<SrcType, DestType>
			{
				typedef EnumeratorWrapper<SrcType, DestType>	base;

			private:
				shared_ptr<LifeAssurance>	_assurance;

			public:
				CastProxy(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<LifeAssurance>& assurance)
					: base(srcEnumerator, &CastProxy::Cast, InstanceOfPredicate<typename GetSharedPtrParam<DestType>::ValueT>()), _assurance(assurance)
				{ }

			private:
				static DestType Cast(ConstSrcTypeRef src) { return dynamic_caster(src); }
			};

		private:
			SrcEnumeratorPtr			_srcEnumerator;
			shared_ptr<LifeAssurance>	_assurance; // In order to control the lifetime of the assurance if necessary

		public:
			EnumeratorCaster(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<LifeAssurance>& assurance)
				: _srcEnumerator(srcEnumerator), _assurance(assurance)
			{ }

			operator SrcEnumeratorPtr() const
			{ return make_shared<Proxy>(_srcEnumerator, _assurance); }

			template < typename DestType >
			operator shared_ptr<IEnumerator<DestType> > () const
			{ return make_shared<CastProxy<DestType> >(_srcEnumerator, _assurance); }
		};


		template < typename T, bool IsEnumerator_ = IsEnumerator<T>::Value >
		struct EnumeratorGetter
		{
			typedef EnumeratorCaster<typename T::ItemType, T>	EnumeratorPtrType;

			static EnumeratorPtrType Get(const shared_ptr<T>& obj)
			{ return EnumeratorPtrType(obj->GetEnumerator(), obj); }
		};

		template < typename T >
		struct EnumeratorGetter<T, true>
		{
			typedef EnumeratorCaster<typename T::ItemType, int>	EnumeratorPtrType;

			static EnumeratorPtrType Get(const shared_ptr<T>& obj)
			{ return EnumeratorPtrType(obj, null); }
		};

		template < typename ItemDeclFunc >
		struct ItemEnumeratorPtr : public shared_ptr<IEnumerator<typename stingray::Detail::GetItemTypeFromItemDecl<ItemDeclFunc>::ValueT> >
		{
			typedef shared_ptr<IEnumerator<typename stingray::Detail::GetItemTypeFromItemDecl<ItemDeclFunc>::ValueT> >	base;

			ItemEnumeratorPtr(const base& ptr) : base(ptr) { }
		};


		inline bool ForEach_ItemFilter(bool dummy, bool val) { return val; }
		inline bool ForEach_ItemFilter(bool dummy) { return true; }
	}

	template < typename T >
	typename Detail::EnumeratorGetter<T>::EnumeratorPtrType GetEnumeratorCaster(const shared_ptr<T>& obj)
	{ return Detail::EnumeratorGetter<T>::Get(obj); }


#define IN ,
#define WHERE ,
#define FOR_EACH__IMPL(ItemDecl_, SomethingToEnumerate_, ...) \
		for (bool __broken__ = false; !__broken__; __broken__ = true) \
			for (::stingray::Detail::ItemEnumeratorPtr<void(*)(ItemDecl_)> en(::stingray::GetEnumeratorCaster(SomethingToEnumerate_)); \
				 en && en->Valid() && !__broken__; \
				 en->Next()) \
				 for (bool __dummy_bool__ = true; __dummy_bool__ && !__broken__; ) \
					 for (ItemDecl_ = en->Get(); (__dummy_bool__ && ((__dummy_bool__ = false) == false) && ::stingray::Detail::ForEach_ItemFilter(true, ##__VA_ARGS__) && (__broken__ = true)); __broken__ = false)

	/** @} */

}

#endif