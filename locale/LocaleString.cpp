#include <stingray/toolkit/locale/LocaleString.h>

#include <stingray/toolkit/locale/StringCodec.h>
#include <stingray/settings/Serialization.h>

namespace stingray {


	LocaleString::LocaleString(): TextEncoding(Encoding::ISO_8859_1), Text()
	{ }

	LocaleString::LocaleString(Encoding encoding, const std::string& text): TextEncoding(encoding), Text(text)
	{ }

	LocaleString::LocaleString(const LocaleString& l) : TextEncoding(l.TextEncoding), Text(l.Text)
	{ }

	LocaleString::~LocaleString()
	{ }

	void LocaleString::Serialize(ObjectOStream & ar) const
	{
		ar.Serialize("encoding", TextEncoding);
		ar.Serialize("text", Text);
	}

	void LocaleString::Deserialize(ObjectIStream & ar)
	{
		ar.Deserialize("encoding", TextEncoding);
		ar.Deserialize("text", Text);
	}

	bool LocaleString::operator<(const LocaleString& other) const
	{ return Compare(*this, other) < 0; }

	bool LocaleString::operator==(const LocaleString& other) const
	{
		if (TextEncoding == other.TextEncoding) //encoding matched
			return Text == other.Text;

		if (Text.size() != other.Text.size())
			return false;

		return Compare(*this, other) == 0;
	}

	bool LocaleString::operator!=(const LocaleString& other) const
	{ return !(*this == other); }

	std::string LocaleString::ToString() const
	{ return StringCodec::ToUtf8(*this); }

	LocaleString LocaleString::FromUtf8(const std::string &str)
	{ return StringCodec::FromUtf8(str); }

	int LocaleString::Compare(const LocaleString& a, const LocaleString &b) const
	{ return StringCodec::Compare(a, b); }


}


