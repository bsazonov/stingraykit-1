// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/Unicode.h>

#include <stingraykit/CheckedDelete.h>
#include <stingraykit/exception.h>
#include <stingraykit/string/ToString.h>

#ifdef HAVE_ICU_I18N
#	include <unicode/unistr.h>
#endif


namespace stingray
{

#ifdef HAVE_ICU_I18N

	UnicodeCollator::UnicodeCollator()
	{
		UErrorCode success = U_ZERO_ERROR;
		_collator.reset(icu::Collator::createInstance(success));
		STINGRAYKIT_CHECK(success != U_FILE_ACCESS_ERROR, "file requested by ICU was not found, please install icudt53l.dat to /usr/share/icu");
		STINGRAYKIT_CHECK(U_SUCCESS(success), "creating collator failed, error: " + ToString(success));
		SetCaseSensitivity(true);
	}


	UnicodeCollator::~UnicodeCollator()
	{ }


	void UnicodeCollator::SetCaseSensitivity(bool sensitive)
	{
		_caseSensitive = sensitive;
		_collator->setStrength(sensitive ? icu::Collator::TERTIARY : icu::Collator::SECONDARY);
	}


	int UnicodeCollator::Compare(const std::string& str1, const std::string& str2) const
	{
		CompileTimeAssert<(UCOL_EQUAL == 0)>	Error_invalid_equal_const;
		CompileTimeAssert<(UCOL_LESS < 0)>		Error_invalid_less_const;
		CompileTimeAssert<(UCOL_GREATER > 0)>	Error_invalid_greater_const;

		UErrorCode success = U_ZERO_ERROR;
		UCollationResult r = _collator->compareUTF8(str1, str2, success);
		STINGRAYKIT_CHECK(U_SUCCESS(success), "compareUTF8 failed, error: " + ToString(success));

		return r;
	}


	std::string Utf8ToLower(const std::string& str)
	{
		icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(str).toLower();
		std::string r;
		ustr.toUTF8String(r);
		return r;
	}

#else

	UnicodeCollator::UnicodeCollator(): _caseSensitive(true) { }


	UnicodeCollator::~UnicodeCollator() { }


	void UnicodeCollator::SetCaseSensitivity(bool sensitive)
	{ _caseSensitive = sensitive; }


	int UnicodeCollator::Compare(const std::string &str1, const std::string &str2) const
	{
		return _caseSensitive? str1.compare(str2): Utf8ToLower(str1).compare(Utf8ToLower(str2));
	}


	std::string Utf8ToLower(const std::string& str)
	{
		std::string result;
		result.reserve(str.size());
		for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		{
			if (*it >= 'A' && *it <= 'Z')
				result.push_back(*it + 'a' - 'A');
			else if ((u8)*it == 0xD0)
			{
				u16 c0 = (u8)*it & 0x1F;
				STINGRAYKIT_CHECK(++it != str.end(), "Malformed utf-8!");
				u16 c = (c0 << 6) | ((u8)*it & 0x3F);
				if (c >= 0x410 && c <= 0x42F)
					c += 0x20;
				else if (c >= 0x401 && c <= 0x40f)
					c += 0x50;
				result.push_back(0xD0 | (c >> 6));
				result.push_back((c & 0x3F) | 0x80);
			}
			else
				result.push_back(*it);
		}
		return result;
	}

#endif

}
