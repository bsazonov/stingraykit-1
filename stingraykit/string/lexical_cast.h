#ifndef STINGRAYKIT_STRING_LEXICAL_CAST_H
#define STINGRAYKIT_STRING_LEXICAL_CAST_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/ToString.h>

namespace stingray
{

	namespace Detail
	{

		template< typename To, typename From >
		struct LexicalCast;


		template< typename To >
		struct LexicalCast<To, std::string>
		{
			static To Do(const std::string& from)
			{ return FromString<To>(from); }
		};


		template< typename From >
		struct LexicalCast<std::string, From>
		{
			static std::string Do(const From& from)
			{ return ToString(from); }
		};


		template < >
		struct LexicalCast<std::string, std::string>
		{
			static std::string Do(const std::string& from)
			{ return from; }
		};

	}


	template < typename To, typename From >
	To lexical_cast(const From& from)
	{ return Detail::LexicalCast<To, From>::Do(from); }


	namespace Detail
	{

		template < typename From >
		class LexicalCasterProxy
		{
		private:
			From	_from;

		public:
			explicit LexicalCasterProxy(const From& from)
				: _from(from)
			{ }

			template < typename To >
			operator To() const
			{ return lexical_cast<To>(_from); }
		};

	}


	template < typename From >
	Detail::LexicalCasterProxy<From> lexical_caster(const From& from)
	{ return Detail::LexicalCasterProxy<From>(from); }

}

#endif