#ifndef STINGRAY_TOOLKIT_COLLECTIONS_IDICTIONARY_H
#define STINGRAY_TOOLKIT_COLLECTIONS_IDICTIONARY_H


#include <utility>

#include <stingray/toolkit/collection/ICollection.h>
#include <stingray/toolkit/collection/IEnumerable.h>
#include <stingray/toolkit/StringUtils.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ >
	struct KeyValuePair
	{
		typedef KeyType_	KeyType;
		typedef ValueType_	ValueType;

		KeyType		Key;
		ValueType	Value;

		KeyValuePair()
			: Key(), Value()
		{ }

		KeyValuePair(const KeyType& key, const ValueType& value)
			: Key(key), Value(value)
		{ }

		KeyValuePair(const std::pair<KeyType, ValueType>& pair)
			: Key(pair.first), Value(pair.second)
		{ }

		KeyType GetKey() const			{ return Key; }
		ValueType GetValue() const		{ return Value; }

		std::string ToString() const	{ return StringBuilder() % Key % " -> " % Value; }
	};

	template < typename KeyType_, typename ValueType_ >
	struct IReadonlyDictionary :
		public virtual ICollection<KeyValuePair<KeyType_, ValueType_> >,
		public virtual IReversableEnumerable<KeyValuePair<KeyType_, ValueType_> >
	{
		typedef KeyType_							KeyType;
		typedef ValueType_							ValueType;
		typedef KeyValuePair<KeyType, ValueType>	PairType;

		virtual ~IReadonlyDictionary() { }

		virtual ValueType Get(const KeyType& key) const = 0;

		virtual bool ContainsKey(const KeyType& key) const = 0;

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			if (!ContainsKey(key))
				return false;
			outValue = Get(key);
			return true;
		}
	};


	template < typename T >
	struct InheritsIReadonlyDictionary : public Inherits2ParamTemplate<T, IReadonlyDictionary>
	{ };


	template < typename KeyType_, typename ValueType_ >
	struct IDictionary :
		public virtual IReadonlyDictionary<KeyType_, ValueType_>
	{
		typedef KeyType_							KeyType;
		typedef ValueType_							ValueType;
		typedef KeyValuePair<KeyType, ValueType>	PairType;

		virtual ~IDictionary() { }

		virtual void Set(const KeyType& key, const ValueType& value) = 0;

		virtual void Remove(const KeyType& key) = 0;

		virtual bool TryRemove(const KeyType& key)
		{
			if (!this->ContainsKey(key))
				return false;
			Remove(key);
			return true;
		}

		virtual void Clear() = 0;
	};


	template<typename EnumeratorType>
	shared_ptr<IEnumerator<typename EnumeratorType::PairType::KeyType> > KeysEnumerator(const shared_ptr<EnumeratorType>& enumerator)
	{
		typedef typename EnumeratorType::PairType PairType;
		return WrapEnumerator(enumerator, &PairType::GetKey);
	}

	template<typename EnumerableType>
	shared_ptr<IEnumerable<typename EnumerableType::PairType::KeyType> > KeysEnumerable(const shared_ptr<EnumerableType>& enumerable)
	{
		typedef typename EnumerableType::PairType PairType;
		return WrapEnumerable(enumerable, &PairType::GetKey);
	}

	template<typename EnumeratorType>
	shared_ptr<IEnumerator<typename EnumeratorType::PairType::ValueType> > ValuesEnumerator(const shared_ptr<EnumeratorType>& enumerator)
	{
		typedef typename EnumeratorType::PairType PairType;
		return WrapEnumerator(enumerator, &PairType::GetValue);
	}

	template<typename EnumerableType>
	shared_ptr<IEnumerable<typename EnumerableType::PairType::ValueType> > ValuesEnumerable(const shared_ptr<EnumerableType>& enumerable)
	{
		typedef typename EnumerableType::PairType PairType;
		return WrapEnumerable(enumerable, &PairType::GetValue);
	}
	/** @} */

}


#endif