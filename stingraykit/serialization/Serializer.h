#ifndef STINGRAYKIT_SERIALIZATION_SERIALIZER_H
#define STINGRAYKIT_SERIALIZATION_SERIALIZER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/optional.h>

namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	template < typename Tag, typename T >
	struct Serialization;

	template < typename Tag, typename T >
	struct Serializer
	{
		const T&	Object;

		explicit Serializer(const T& object) : Object(object) { }

		template < typename OStream_ >
		void SerializeAsValue(OStream_& ar) const
		{ Serialization<Tag, T>::Serialize(ar, Object); }
	};

	template < typename Tag, typename T >
	struct Serializer<Tag, optional<T> >
	{
		const optional<T>&		Object;

		explicit Serializer(const optional<T>& object) : Object(object) { }

		template < typename OStream_ >
		void SerializeAsValue(OStream_& ar) const
		{
			if (Object)
				ar.Serialize(Serializer<Tag, T>(*Object));
			else
				ar.Serialize(optional<T>());
		}
	};

	template < typename Tag, typename T >
	Serializer<Tag, T> MakeSerializer(const T& object) { return Serializer<Tag, T>(object); }


	namespace Detail
	{

		template < typename T, typename Deserializer_ >
		struct OptionalDeserializationHelper
		{
			optional<T>		Object;
			optional<T>&	Target;

			OptionalDeserializationHelper() : Object(), Target(Object) { }

			explicit OptionalDeserializationHelper(optional<T>& target) : Object(), Target(target) { }

			const OptionalDeserializationHelper& operator = (const OptionalDeserializationHelper& other) const
			{
				Target = other.Object;
				return *this;
			}

			template < typename IStream_ >
			void DeserializeAsValue(IStream_& ar)
			{
				T object;
				ar.Deserialize(*Deserializer_(object));
				Object = object;
			}
		};

	}


	template < typename Tag, typename T >
	struct Deserializer
	{
		T&		Object;

		explicit Deserializer(T& object) : Object(object) { }

		template < typename IStream_ >
		void DeserializeAsValue(IStream_& ar)
		{ Serialization<Tag, T>::Deserialize(ar, Object); }

		Deserializer& operator * () { return *this; }
	};

	template < typename Tag, typename T >
	class Deserializer<Tag, optional<T> >
	{
		typedef Detail::OptionalDeserializationHelper<T, Deserializer<Tag, T> > DeserializationHelper;

		optional<DeserializationHelper>		_helper;

	public:
		explicit Deserializer(optional<T>& object) : _helper(DeserializationHelper(object)) { }

		optional<DeserializationHelper>& operator * () { return _helper; }
	};

	template < typename Tag, typename T >
	Deserializer<Tag, T> MakeDeserializer(T& object) { return Deserializer<Tag, T>(object); }


	struct DefaultSerializationTag;

	template < typename Object >
	struct Serialization<DefaultSerializationTag, Object>
	{
		template < typename OStream >
		static void Serialize(OStream& ar, const Object& object)
		{ ar.Serialize(object); }

		template < typename IStream >
		static void Deserialize(IStream& ar, Object& object)
		{ ar.Deserialize(object); }
	};

	/** @} */

}

#endif
