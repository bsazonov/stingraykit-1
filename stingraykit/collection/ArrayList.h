#ifndef STINGRAYKIT_COLLECTION_ARRAYLIST_H
#define STINGRAYKIT_COLLECTION_ARRAYLIST_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumeratorFromStlContainer.h>
#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/IList.h>

#include <algorithm>
#include <vector>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	class ArrayList : public virtual IList<T>
	{
	public:
		typedef typename IList<T>::ValueType				ValueType;

	private:
		typedef std::vector<ValueType>						VectorType;
		STINGRAYKIT_DECLARE_PTR(VectorType);

		struct Holder
		{
			VectorTypePtr		Items;
			Holder(const VectorTypePtr& items) : Items(items) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		struct ReverseEnumerable : public virtual IEnumerable<ValueType>
		{
			HolderPtr		_holder;

			ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
			{ return EnumeratorFromStlIterators(_holder->Items->rbegin(), _holder->Items->rend(), _holder); }
		};

	private:
		VectorTypePtr			_items;
		mutable HolderWeakPtr	_itemsHolder;

	public:
		ArrayList()
			:	_items(make_shared<VectorType>())
		{ }

		ArrayList(shared_ptr<IEnumerator<ValueType> > enumerator)
			:	_items(make_shared<VectorType>())
		{ Enumerable::ForEach(enumerator, bind(&ArrayList::Add, this, _1)); }

		ArrayList(shared_ptr<IEnumerable<ValueType> > enumerable)
			:	_items(make_shared<VectorType>())
		{ Enumerable::ForEach(enumerable, bind(&ArrayList::Add, this, _1)); }

		ArrayList(const ArrayList& other)
		{ CopyItems(other._items); }

		ArrayList& operator = (const ArrayList& other)
		{ CopyItems(other._items); return *this; }

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{ return EnumeratorFromStlContainer(*_items, GetItemsHolder()); }

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{ return make_shared<ReverseEnumerable>(GetItemsHolder()); }

		virtual size_t GetCount() const
		{ return _items->size(); }

		virtual bool IsEmpty() const
		{ return _items->empty(); }

		virtual optional<size_t> IndexOf(const ValueType& value) const
		{
			const typename VectorType::const_iterator it = std::find(_items->begin(), _items->end(), value);
			return it == _items->end() ? null : optional<size_t>(it - _items->begin());
		}

		virtual ValueType Get(size_t index) const
		{
			STINGRAYKIT_CHECK(index < _items->size(), IndexOutOfRangeException(index, _items->size()));
			return (*_items)[index];
		}

		virtual void Add(const ValueType& value)
		{
			CopyOnWrite();
			_items->push_back(value);
		}

		virtual void Set(size_t index, const ValueType& value)
		{
			STINGRAYKIT_CHECK(index < _items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();
			(*_items)[index] = value;
		}

		virtual void Insert(size_t index, const ValueType& value)
		{
			STINGRAYKIT_CHECK(index <= _items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();

			typename VectorType::iterator it = _items->begin();
			std::advance(it, index);
			_items->insert(it, value);
		}

		virtual void RemoveAt(size_t index)
		{
			STINGRAYKIT_CHECK(index < _items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();
			_items->erase(_items->begin() + index);
		}

		virtual size_t RemoveAll(const function<bool (const ValueType&)>& pred)
		{
			CopyOnWrite();
			const typename VectorType::iterator it = std::remove_if(_items->begin(), _items->end(), pred);
			const size_t ret = std::distance(it, _items->end());
			_items->erase(it, _items->end());
			return ret;
		}

		virtual void Clear()
		{
			CopyOnWrite();
			_items->clear();
		}

	private:
		void CopyItems(const VectorTypePtr& items)
		{
			_items = make_shared<VectorType>(*items);
			_itemsHolder.reset();
		}

		HolderPtr GetItemsHolder() const
		{
			HolderPtr itemsHolder = _itemsHolder.lock();

			if (!itemsHolder)
				_itemsHolder = (itemsHolder = make_shared<Holder>(_items));

			return itemsHolder;
		}

		void CopyOnWrite()
		{
			if (_itemsHolder.lock())
				CopyItems(_items);
		}
	};

	/** @} */

}

#endif
