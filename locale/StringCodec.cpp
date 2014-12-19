#include <stingray/toolkit/locale/StringCodec.h>

#include <stingray/toolkit/exception.h>


namespace stingray
{

	static const u32 InvalidCharacter = ~0;


	static u32 Unpack_ISO_8859_1(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		return c;
	}

	static u32 Unpack_ISO_8859_2(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0_100[] =
		{
			0x00a0, 0x0104, 0x02d8, 0x0141, 0x00a4, 0x013d, 0x015a, 0x00a7, 0x00a8, 0x0160, 0x015e, 0x0164, 0x0179, 0x00ad, 0x017d, 0x017b,
			0x00b0, 0x0105, 0x02db, 0x0142, 0x00b4, 0x013e, 0x015b, 0x02c7, 0x00b8, 0x0161, 0x015f, 0x0165, 0x017a, 0x02dd, 0x017e, 0x017c,
			0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
			0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7, 0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
			0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
			0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7, 0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,
		};
		return c < 0xa0? c: table_a0_100[c - 0xa0];
	}

	static u32 Unpack_ISO_8859_3(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0x0126, 0x02d8, 0x00a3, 0x00a4, 0xffff, 0x0124, 0x00a7, 0x00a8, 0x0130, 0x015e, 0x011e, 0x0134, 0x00ad, 0xffff, 0x017b,
			0x00b0, 0x0127, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x0125, 0x00b7, 0x00b8, 0x0131, 0x015f, 0x011f, 0x0135, 0x00bd, 0xffff, 0x017c,
			0x00c0, 0x00c1, 0x00c2, 0xffff, 0x00c4, 0x010a, 0x0108, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
			0xffff, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x0120, 0x00d6, 0x00d7, 0x011c, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x016c, 0x015c, 0x00df,
			0x00e0, 0x00e1, 0x00e2, 0xffff, 0x00e4, 0x010b, 0x0109, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
			0xffff, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x0121, 0x00f6, 0x00f7, 0x011d, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x016d, 0x015d, 0x02d9,
		};
		if (c < 0xa0)
			return c;
		u16 r = table_a0[c - 0xa0];
		return r != 0xffff? r: InvalidCharacter;
	}

	static u32 Unpack_ISO_8859_4(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0x0104, 0x0138, 0x0156, 0x00a4, 0x0128, 0x013b, 0x00a7, 0x00a8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00ad, 0x017d, 0x00af,
			0x00b0, 0x0105, 0x02db, 0x0157, 0x00b4, 0x0129, 0x013c, 0x02c7, 0x00b8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014a, 0x017e, 0x014b,
			0x0100, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x012e, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x0116, 0x00cd, 0x00ce, 0x012a,
			0x0110, 0x0145, 0x014c, 0x0136, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x0172, 0x00da, 0x00db, 0x00dc, 0x0168, 0x016a, 0x00df,
			0x0101, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x012f, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x0117, 0x00ed, 0x00ee, 0x012b,
			0x0111, 0x0146, 0x014d, 0x0137, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x0173, 0x00fa, 0x00fb, 0x00fc, 0x0169, 0x016b, 0x02d9,
		};
		return c < 0xa0? c: table_a0[c - 0xa0];
	}

	static u32 Unpack_ISO_8859_5(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 0x040a, 0x040b, 0x040c, 0x00ad, 0x040e, 0x040f,
			0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
			0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
			0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
			0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
			0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 0x0459, 0x045a, 0x045b, 0x045c, 0x00a7, 0x045e, 0x045f,
		};
		return c < 0xa0? c: table_a0[c - 0xa0];
	}

	static u32 Unpack_ISO_8859_6(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0xffff, 0xffff, 0xffff, 0x00a4, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x060c, 0x00ad, 0xffff, 0xffff,
			0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x061b, 0xffff, 0xffff, 0xffff, 0x061f,
			0xffff, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062a, 0x062b, 0x062c, 0x062d, 0x062e, 0x062f,
			0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x063a, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
			0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649, 0x064a, 0x064b, 0x064c, 0x064d, 0x064e, 0x064f,
			0x0650, 0x0651, 0x0652, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		};
		if (c < 0xa0)
			return c;
		u16 r = table_a0[c - 0xa0];
		return r != 0xffff? r: InvalidCharacter;
	}

	static u32 Unpack_ISO_8859_7(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0x2018, 0x2019, 0x00a3, 0x20ac, 0x20af, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x037a, 0x00ab, 0x00ac, 0x00ad, 0xfffd, 0x2015,
			0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x0384, 0x0385, 0x0386, 0x00b7, 0x0388, 0x0389, 0x038a, 0x00bb, 0x038c, 0x00bd, 0x038e, 0x038f,
			0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
			0x03a0, 0x03a1, 0xfffd, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7, 0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
			0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
			0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce, 0xfffd,
		};
		if (c < 0xa0)
			return c;
		u16 r = table_a0[c - 0xa0];
		return r != 0xffff? r: InvalidCharacter;
	}

	static u32 Unpack_ISO_8859_8(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0xfffd, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00d7, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
			0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00f7, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0xfffd,
			0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd,
			0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0x2017,
			0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05d4, 0x05d5, 0x05d6, 0x05d7, 0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05dd, 0x05de, 0x05df,
			0x05e0, 0x05e1, 0x05e2, 0x05e3, 0x05e4, 0x05e5, 0x05e6, 0x05e7, 0x05e8, 0x05e9, 0x05ea, 0xfffd, 0xfffd, 0x200e, 0x200f, 0xfffd,
		};
		if (c < 0xa0)
			return c;
		u16 r = table_a0[c - 0xa0];
		return r != 0xffff? r: InvalidCharacter;
	}

	static u32 Unpack_ISO_8859_9(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_d0[] =
		{
			0x011e, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0130, 0x015e, 0x00df,
			0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
			0x011f, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0131, 0x015f, 0x00ff,
		};
		return c < 0xd0? c: table_d0[c - 0xd0];
	}

	static u32 Unpack_ISO_8859_10(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0x0104, 0x0112, 0x0122, 0x012a, 0x0128, 0x0136, 0x00a7, 0x013b, 0x0110, 0x0160, 0x0166, 0x017d, 0x00ad, 0x016a, 0x014a,
			0x00b0, 0x0105, 0x0113, 0x0123, 0x012b, 0x0129, 0x0137, 0x00b7, 0x013c, 0x0111, 0x0161, 0x0167, 0x017e, 0x2015, 0x016b, 0x014b,
			0x0100, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x012e, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x0116, 0x00cd, 0x00ce, 0x00cf,
			0x00d0, 0x0145, 0x014c, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x0168, 0x00d8, 0x0172, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
			0x0101, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x012f, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x0117, 0x00ed, 0x00ee, 0x00ef,
			0x00f0, 0x0146, 0x014d, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x0169, 0x00f8, 0x0173, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x0138,
		};
		return c < 0xa0? c: table_a0[c - 0xa0];
	}

	static u32 Unpack_ISO_8859_11(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		if (c <= 0xa0)
			return c;
		if (c == 0xdb || c == 0xdc || c == 0xde)
			return InvalidCharacter;
		return 0x0d60 + c;
	}

	static u32 Unpack_ISO_8859_13(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0x201d, 0x00a2, 0x00a3, 0x00a4, 0x201e, 0x00a6, 0x00a7, 0x00d8, 0x00a9, 0x0156, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00c6,
			0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x201c, 0x00b5, 0x00b6, 0x00b7, 0x00f8, 0x00b9, 0x0157, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00e6,
			0x0104, 0x012e, 0x0100, 0x0106, 0x00c4, 0x00c5, 0x0118, 0x0112, 0x010c, 0x00c9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012a, 0x013b,
			0x0160, 0x0143, 0x0145, 0x00d3, 0x014c, 0x00d5, 0x00d6, 0x00d7, 0x0172, 0x0141, 0x015a, 0x016a, 0x00dc, 0x017b, 0x017d, 0x00df,
			0x0105, 0x012f, 0x0101, 0x0107, 0x00e4, 0x00e5, 0x0119, 0x0113, 0x010d, 0x00e9, 0x017a, 0x0117, 0x0123, 0x0137, 0x012b, 0x013c,
			0x0161, 0x0144, 0x0146, 0x00f3, 0x014d, 0x00f5, 0x00f6, 0x00f7, 0x0173, 0x0142, 0x015b, 0x016b, 0x00fc, 0x017c, 0x017e, 0x2019,
		};
		return c < 0xa0? c: table_a0[c - 0xa0];
	}

	static u32 Unpack_ISO_8859_14(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0x1e02, 0x1e03, 0x00a3, 0x010a, 0x010b, 0x1e0a, 0x00a7, 0x1e80, 0x00a9, 0x1e82, 0x1e0b, 0x1ef2, 0x00ad, 0x00ae, 0x0178,
			0x1e1e, 0x1e1f, 0x0120, 0x0121, 0x1e40, 0x1e41, 0x00b6, 0x1e56, 0x1e81, 0x1e57, 0x1e83, 0x1e60, 0x1ef3, 0x1e84, 0x1e85, 0x1e61,
			0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
			0x0174, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x1e6a, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x0176, 0x00df,
			0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
			0x0175, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x1e6b, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x0177, 0x00ff,
		};
		return c < 0xa0? c: table_a0[c - 0xa0];
	}

	static u32 Unpack_ISO_8859_15(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0_c0[] =
		{
			0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x20ac, 0x00a5, 0x0160, 0x00a7, 0x0161, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
			0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x017d, 0x00b5, 0x00b6, 0x00b7, 0x017e, 0x00b9, 0x00ba, 0x00bb, 0x0152, 0x0153, 0x0178, 0x00bf,
		};
		return (c < 0xa0 || c >= 0xc0)? c: table_a0_c0[c - 0xa0];
	}

	static u32 Unpack_ISO_8859_16(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		static const u16 table_a0[] =
		{
			0x00a0, 0x0104, 0x0105, 0x0141, 0x20ac, 0x201e, 0x0160, 0x00a7, 0x0161, 0x00a9, 0x0218, 0x00ab, 0x0179, 0x00ad, 0x017a, 0x017b,
			0x00b0, 0x00b1, 0x010c, 0x0142, 0x017d, 0x201d, 0x00b6, 0x00b7, 0x017e, 0x010d, 0x0219, 0x00bb, 0x0152, 0x0153, 0x0178, 0x017c,
			0x00c0, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0106, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
			0x0110, 0x0143, 0x00d2, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x015a, 0x0170, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0118, 0x021a, 0x00df,
			0x00e0, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x0107, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
			0x0111, 0x0144, 0x00f2, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x015b, 0x0171, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0119, 0x021b, 0x00ff,
		};
		return c < 0xa0? c: table_a0[c - 0xa0];
	}

	static u32 Unpack_ISO_10646_utf8(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		unsigned char c0 = (unsigned char)*i++;
		if (c0 <= 0x7f)
			return c0;

		if (c0 == 0xc0 || c0 == 0xc1 || c0 >= 0xf5 || i == end)
			return InvalidCharacter;

		unsigned char c1 = (unsigned char)*i++;
		if (c0 >= 0xc2 && c0 <= 0xdf)
			return ((c0 & 0x1f) << 6) | (c1 & 0x3f);

		if (i == end)
			return InvalidCharacter;

		unsigned char c2 = (unsigned char)*i++;
		if (c0 >= 0xe0 && c0 <= 0xef)
			return ((c0 & 0x0f) << 12) | ((c1 & 0x3f) << 6) | (c2 & 0x3f);

		if (i == end)
			return InvalidCharacter;

		unsigned char c3 = (unsigned char)*i++;
		if (c0 >= 0xf0 && c0 <= 0xf4)
			return ((c0 & 0x07) << 18) | ((c1 & 0x3f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);

		return InvalidCharacter;
	}

	static u32 Unpack_ISO_10646_utf16le(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		unsigned char c0 = (unsigned char)*i++;
		if (i == end)
			return InvalidCharacter;

		unsigned char c1 = (unsigned char)*i++;
		if (c0 < 0xd8 || c0 > 0xdf)
			return (c0 << 8) | c1;

		if (c0 > 0xdb || i == end)
			return InvalidCharacter;

		unsigned char c2 = (unsigned char)*i++;
		if (i == end)
			return InvalidCharacter;

		unsigned char c3 = (unsigned char)*i++;
		if (c2 < 0xdc || c2 > 0xdf)
			return InvalidCharacter;

		return ((c0 & 0x03) << 18) | (c1 << 10) | ((c2 & 0x03) << 8) | c3;
	}

	static u32 Unpack_ISO_10646_utf16be(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		unsigned char c0 = (unsigned char)*i++;
		if (i == end)
			return InvalidCharacter;

		unsigned char c1 = (unsigned char)*i++;
		if (c1 < 0xd8 || c1 > 0xdf)
			return (c1 << 8) | c0;

		if (c1 > 0xdb || i == end)
			return InvalidCharacter;

		unsigned char c2 = (unsigned char)*i++;
		if (i == end)
			return InvalidCharacter;

		unsigned char c3 = (unsigned char)*i++;
		if (c3 < 0xdc || c3 > 0xdf)
			return InvalidCharacter;

		return ((c1 & 0x03) << 18) | (c0 << 10) | ((c3 & 0x03) << 8) | c2;
	}

	void StringCodec::PackUtf8(std::string &dst, u32 ucs, u32 replacement)
	{
		if (ucs < 0x80)
		{
			dst += (char)ucs;
		}
		else if (ucs < 0x800)
		{
			dst += (char) ((ucs >> 6) | 0xc0);
			dst += (char) ((ucs & 0x3f) | 0x80);
		}
		else if (ucs < 0x10000)
		{
			dst += (char)((ucs >> 12) | 0xe0);
			dst += (char)(((ucs & 0x0fc0) >> 6) | 0x80);
			dst += (char)((ucs & 0x003f) | 0x80);
		}
		else if (ucs < 0x110000) //actually 0x200000, but unicode still not reached this limit.
		{
			dst += (char)((ucs >> 18) | 0xf0);
			dst += (char)(((ucs & 0x03f000) >> 12) | 0x80);
			dst += (char)(((ucs & 0x000fc0) >> 6) | 0x80);
			dst += (char)( (ucs & 0x00003f) | 0x80);
		}
		else
			dst += replacement;
	}

	static u32 Unpack_ISO_10646(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u32 ucs = 0;
		for(unsigned n = 0; n < 4; ++n)
		{
			if (i == end)
				return InvalidCharacter;
			ucs <<= 8;
			ucs |= (u8)*i++;
		}
		return ucs < 0x110000? ucs: InvalidCharacter;
	}

	StringCodec::UnpackFunc StringCodec::GetUnpackFunc(const Encoding &encoding)
	{
		switch(encoding)
		{
		case Encoding::ISO_8859_1:	return &Unpack_ISO_8859_1;
		case Encoding::ISO_8859_2:	return &Unpack_ISO_8859_2;
		case Encoding::ISO_8859_3:	return &Unpack_ISO_8859_3;
		case Encoding::ISO_8859_4:	return &Unpack_ISO_8859_4;
		case Encoding::ISO_8859_5:	return &Unpack_ISO_8859_5;
		case Encoding::ISO_8859_6:	return &Unpack_ISO_8859_6;
		case Encoding::ISO_8859_7:	return &Unpack_ISO_8859_7;
		case Encoding::ISO_8859_8:	return &Unpack_ISO_8859_8;
		case Encoding::ISO_8859_9:	return &Unpack_ISO_8859_9;
		case Encoding::ISO_8859_10:	return &Unpack_ISO_8859_10;
		case Encoding::ISO_8859_11:	return &Unpack_ISO_8859_11;
		case Encoding::ISO_8859_13:	return &Unpack_ISO_8859_13;
		case Encoding::ISO_8859_14:	return &Unpack_ISO_8859_14;
		case Encoding::ISO_8859_15:	return &Unpack_ISO_8859_15;
		case Encoding::ISO_8859_16:	return &Unpack_ISO_8859_16;
		case Encoding::ISO_10646:	return &Unpack_ISO_10646;
		case Encoding::ISO_10646_utf8: return &Unpack_ISO_10646_utf8;
		case Encoding::ISO_10646_utf16LE: return &Unpack_ISO_10646_utf16le;
		case Encoding::ISO_10646_utf16BE: return &Unpack_ISO_10646_utf16be;
		default:					return NULL;
		}
	}

	std::string StringCodec::ToUtf8(const LocaleString &src, u32 replacement)
	{
		if (src.TextEncoding == Encoding::ISO_10646_utf8)
			return src.Text;

		UnpackFunc unpack = GetUnpackFunc(src.TextEncoding);
		if (!unpack)
			TOOLKIT_THROW(std::runtime_error("conversion from " + src.TextEncoding.ToString() + " not supported"));

		std::string r;
		r.reserve(src.Text.size() * 2);

		for(std::string::const_iterator i = src.Text.begin(), end = src.Text.end(); i != end; )
		{
			u32 ucs = unpack(i, end);
			if (ucs == InvalidCharacter)
				ucs = replacement;
			PackUtf8(r, ucs, replacement);
		}
		r.reserve(r.size()); //«In all other cases, it is taken as a non-binding request to shrink the string capacity»

		return r;
	}

	LocaleString StringCodec::FromUtf8(const std::string &str)
	{
		return LocaleString(Encoding::ISO_10646_utf8, str);
	}

	int StringCodec::Compare(const LocaleString &a, const LocaleString &b)
	{
		//please do not "optimize" it comparing encodings, this is invalid,
		//as it different symbols are differently ordered between unicode and iso
		UnpackFunc unpack_a = GetUnpackFunc(a.TextEncoding), unpack_b = GetUnpackFunc(b.TextEncoding);
		if (!unpack_a)
			TOOLKIT_THROW(std::runtime_error("conversion from " + a.TextEncoding.ToString() + " not supported"));
		if (!unpack_b)
			TOOLKIT_THROW(std::runtime_error("conversion from " + b.TextEncoding.ToString() + " not supported"));

		const std::string &str_a = a.Text, &str_b = b.Text;
		std::string::const_iterator i_a = str_a.begin(), i_b = str_b.begin();
		std::string::const_iterator end_a = str_a.end(), end_b = str_b.end();
		while(i_a != end_a && i_b != end_b)
		{
			int ucs_a = unpack_a(i_a, end_a);
			int ucs_b = unpack_b(i_b, end_b);
			if (ucs_a != ucs_b)
				return ucs_a - ucs_b;
		}
		return (i_a != end_a? 1: 0) + (i_b != end_b? -1: 0);
	}

	static void Pack_ASCII(std::string &str, u32 unicode, char invalid_char_replacement)
	{
		if (unicode < 0x80)
			str += (char)unicode;
		else
			str += invalid_char_replacement;
	}

	static void Pack_CP866(std::string &str, u32 unicode, char invalid_char_replacement)
	{
		if (unicode < 0x80)
			str += (char)unicode;
		else if (unicode >= 0x0410 && unicode <= 0x043f)
			str += (char)(unicode - 0x410 + 0x80);
		else if (unicode >= 0x0440 && unicode <= 0x044f)
			str += (char)(unicode - 0x440 + 0xe0);
		else
		{
			switch(unicode)
			{
			case 0x0401: str += (char)0xf0; break;
			case 0x0451: str += (char)0xf1; break;
			case 0x0404: str += (char)0xf2; break;
			case 0x0454: str += (char)0xf3; break;
			case 0x0407: str += (char)0xf4; break;
			case 0x0457: str += (char)0xf5; break;
			case 0x040e: str += (char)0xf6; break;
			case 0x045e: str += (char)0xf7; break;

			case 0x00b0: str += (char)0xf8; break;
			case 0x2219: str += (char)0xf9; break;
			case 0x00b7: str += (char)0xfa; break;
			case 0x221a: str += (char)0xfb; break;
			case 0x2116: str += (char)0xfc; break;
			case 0x00a4: str += (char)0xfd; break;
			case 0x25a0: str += (char)0xfe; break;
			case 0x00a0: str += (char)0xff; break;

			default: str += invalid_char_replacement;
			}
		}
	}

	StringCodec::PackFunc StringCodec::GetCodePagePackFunc(unsigned code_page)
	{
		switch(code_page)
		{
		case 866:	return &Pack_CP866;
		default:	return &Pack_ASCII;
		}
	}

	static u32 Unpack_ASCII(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		return c < 0x80? c: InvalidCharacter;
	}

	static u32 Unpack_CP866(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;
		if (c < 0x80)
			return c;
		else if (c >= 0x80 && c < 0xb0)
			return 0x410 + c - 0x80;
		else if (c >= 0xe0 && c < 0xf0)
			return 0x440 + c - 0xe0;
		else if (c >= 0xf0)
		{
			static const u32 table_f0[16] =
			{
				0x0401, 0x0451, 0x0404, 0x0454,
				0x0407, 0x0457, 0x040e, 0x045e,
				0x00b0, 0x2219, 0x00B7, 0x221A,
				0x2116, 0x00A4, 0x25A0, 0x00A0,
			};
			return table_f0[c - 0xf0];
		}
		else
			return InvalidCharacter;
	}

	static u32 Unpack_CP1251(std::string::const_iterator &i, const std::string::const_iterator &end)
	{
		u8 c = (u8)*i++;

		static const u16 table_80[] =
		{
			0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
			0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0000, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
			0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, 0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
			0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, 0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
			0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
			0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
			0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
			0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
		};
		return c < 0x80? c: (table_80[c - 0x80]);
	}

	StringCodec::UnpackFunc StringCodec::GetCodePageUnpackFunc(unsigned code_page)
	{
		switch(code_page)
		{
		case 866:	return &Unpack_CP866;
		case 1251:	return &Unpack_CP1251;
		default:	return &Unpack_ASCII;
		}
	}

	LocaleString StringCodec::FromCodePage(const std::string &src, unsigned code_page, u32 invalid_char_replacement)
	{
		UnpackFunc unpack = GetCodePageUnpackFunc(code_page);

		LocaleString r;
		r.TextEncoding = Encoding::ISO_10646_utf8;

		for(std::string::const_iterator i = src.begin(), end = src.end(); i != end; )
		{
			u32 ucs = unpack(i, end);
			if (ucs == InvalidCharacter)
				ucs = invalid_char_replacement;
			PackUtf8(r.Text, ucs, invalid_char_replacement);
		}

		return r;
	}

	std::string StringCodec::ToCodePage(const LocaleString &src, unsigned code_page)
	{
		UnpackFunc unpack = GetUnpackFunc(src.TextEncoding);
		if (!unpack)
			TOOLKIT_THROW("invalid src encoding" + src.TextEncoding.ToString());

		PackFunc pack = GetCodePagePackFunc(code_page); //always valid
		std::string::const_iterator i = src.Text.begin();
		std::string::const_iterator e = src.Text.end();
		std::string result;
		while(i != e)
		{
			u32 unicode = unpack(i, e);
			pack(result, unicode, '?');
		}
		return result;
	}

}
