#ifndef STINGRAYKIT_ENUMMAPPER_H
#define STINGRAYKIT_ENUMMAPPER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <climits>

#include <stingraykit/Singleton.h>
#include <stingraykit/exception.h>


namespace stingray
{


	template < typename SrcEnum_, typename DestEnum_ >
	struct EnumMappingEndNode
	{
		typedef SrcEnum_		SrcEnum;
		typedef DestEnum_		DestEnum;
	};


	template < typename SrcEnum_, typename DestEnum_, SrcEnum_ SrcVal_, DestEnum_ DestVal_, typename Next_ >
	struct EnumMappingNode
	{
		typedef Next_			Next;

		typedef SrcEnum_		SrcEnum;
		typedef DestEnum_		DestEnum;

		static const SrcEnum	SrcVal	= SrcVal_;
		static const DestEnum	DestVal	= DestVal_;
	};


	namespace Detail
	{

		template < typename EnumMapping, bool SourceEnumValue = true >
		struct EnumMappingValueGetter
		{ static const typename EnumMapping::SrcEnum Value = EnumMapping::SrcVal; };


		template < typename EnumMapping >
		struct EnumMappingValueGetter< EnumMapping, false >
		{ static const typename EnumMapping::DestEnum Value = EnumMapping::DestVal; };


		template < typename EnumMapping, bool SourceEnumTraits >
		struct EnumMappingTraits
		{
		private:
			typedef typename EnumMapping::Next NextNode;

			static const int NextMinValue = EnumMappingTraits<NextNode, SourceEnumTraits>::MinValue;
			static const int NextMaxValue = EnumMappingTraits<NextNode, SourceEnumTraits>::MaxValue;
			static const int CurrentValue = static_cast<int>(EnumMappingValueGetter<EnumMapping, SourceEnumTraits>::Value);

		public:
			static const int MinValue = CurrentValue < NextMinValue? CurrentValue : NextMinValue;
			static const int MaxValue = CurrentValue > NextMaxValue? CurrentValue : NextMaxValue;
		};


		template < typename SrcEnum, typename DestEnum, bool SourceEnumTraits >
		struct EnumMappingTraits<EnumMappingEndNode<SrcEnum, DestEnum >, SourceEnumTraits>
		{
			static const int MinValue = ~0;
			static const int MaxValue = ~0;
		};


		template < typename EnumMapping, bool MapSourceEnum >
		class EnumMapperArrayBuilder
		{
			typedef typename If<MapSourceEnum, typename EnumMapping::DestEnum, typename EnumMapping::SrcEnum>::ValueT DestEnum;

		public:
			static void Build(DestEnum* array, bool* mapped, int minValue)
			{
				const int index = static_cast<int>(EnumMappingValueGetter<EnumMapping, MapSourceEnum>::Value) - minValue;

				array[index] = EnumMappingValueGetter<EnumMapping, !MapSourceEnum>::Value;
				mapped[index] = true;

				EnumMapperArrayBuilder<typename EnumMapping::Next, MapSourceEnum>::Build(array, mapped, minValue);
			}
		};


		template < typename SrcEnum_, typename DestEnum_, bool MapSourceEnum >
		class EnumMapperArrayBuilder<EnumMappingEndNode<SrcEnum_, DestEnum_>, MapSourceEnum>
		{
			typedef typename If<MapSourceEnum, DestEnum_, SrcEnum_>::ValueT DestEnum;

		public:
			static void Build(DestEnum*, bool*, int) { }
		};


		template < typename EnumMapping, bool MapSource >
		class EnumMapper
		{
			typedef typename If<MapSource, typename EnumMapping::SrcEnum, typename EnumMapping::DestEnum>::ValueT	SrcEnum;
			typedef typename If<!MapSource, typename EnumMapping::SrcEnum, typename EnumMapping::DestEnum>::ValueT	DestEnum;

			static const int MinValue = EnumMappingTraits<EnumMapping, MapSource>::MinValue;
			static const int MaxValue = EnumMappingTraits<EnumMapping, MapSource>::MaxValue;

			static const int ArraySize = MaxValue - MinValue + 1;

		private:
			DestEnum	_array[ArraySize];
			bool		_mapped[ArraySize];

		public:
			inline EnumMapper()
			{
				std::fill(_mapped, _mapped + ArraySize, false);
				EnumMapperArrayBuilder<EnumMapping, MapSource>::Build(_array, _mapped, MinValue);
			}

			static DestEnum Map(SrcEnum val)
			{ return Singleton<EnumMapper<EnumMapping, MapSource> >::Instance().DoMap(val); }

		private:
			DestEnum DoMap(SrcEnum enumValue)
			{
				const int val = static_cast<int>(enumValue);
				const int index = val - MinValue;
				STINGRAYKIT_CHECK(val >= MinValue && val <= MaxValue, ArgumentException("val"));
				STINGRAYKIT_CHECK(_mapped[index], ArgumentException("val"));
				return _array[index];
			}
		};
	}


	template < typename EnumMapping >
	inline typename EnumMapping::DestEnum MapEnum(typename EnumMapping::SrcEnum val)
	{ return Detail::EnumMapper<EnumMapping, true>::Map(val); }

	template < typename EnumMapping >
	inline typename EnumMapping::SrcEnum UnmapEnum(typename EnumMapping::DestEnum val)
	{ return Detail::EnumMapper<EnumMapping, false>::Map(val); }


#define VD(Num_) SrcEnum_ S##Num_, DestEnum_ D##Num_
#define V(Num_) S##Num_, D##Num_

#define DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(Size_, PrevSize_, ValuesDecl_, Values_ ) \
	template < typename SrcEnum_, typename DestEnum_, SrcEnum_ S1, DestEnum_ D1, ValuesDecl_ > \
	class EnumMapping_##Size_ : public EnumMappingNode<SrcEnum_, DestEnum_, S1, D1, EnumMapping_##PrevSize_<SrcEnum_, DestEnum_, Values_> > { }


	template < typename SrcEnum_, typename DestEnum_, SrcEnum_ S1, DestEnum_ D1 > \
	class EnumMapping_1 : public EnumMappingNode<SrcEnum_, DestEnum_, S1, D1, EnumMappingEndNode<SrcEnum_, DestEnum_> > { };


	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(2, 1, MK_PARAM(VD(2)), MK_PARAM(V(2)));
	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(3, 2, MK_PARAM(VD(2), VD(3)), MK_PARAM(V(2), V(3)));
	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(4, 3, MK_PARAM(VD(2), VD(3), VD(4)), MK_PARAM(V(2), V(3), V(4)));
	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(5, 4, MK_PARAM(VD(2), VD(3), VD(4), VD(5)), MK_PARAM(V(2), V(3), V(4), V(5)));
	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(6, 5, MK_PARAM(VD(2), VD(3), VD(4), VD(5), VD(6)), MK_PARAM(V(2), V(3), V(4), V(5), V(6)));
	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(7, 6, MK_PARAM(VD(2), VD(3), VD(4), VD(5), VD(6), VD(7)), MK_PARAM(V(2), V(3), V(4), V(5), V(6), V(7)));
	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(8, 7, MK_PARAM(VD(2), VD(3), VD(4), VD(5), VD(6), VD(7), VD(8)), MK_PARAM(V(2), V(3), V(4), V(5), V(6), V(7), V(8)));
	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(9, 8, MK_PARAM(VD(2), VD(3), VD(4), VD(5), VD(6), VD(7), VD(8), VD(9)), MK_PARAM(V(2), V(3), V(4), V(5), V(6), V(7), V(8), V(9)));
	DETAIL_STINGRAYKIT_DECLARE_ENUMMAPPING(10, 9, MK_PARAM(VD(2), VD(3), VD(4), VD(5), VD(6), VD(7), VD(8), VD(9), VD(10)), MK_PARAM(V(2), V(3), V(4), V(5), V(6), V(7), V(8), V(9), V(10)));

#undef VD
#undef V


}



#endif
