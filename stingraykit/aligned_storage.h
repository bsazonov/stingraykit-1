#ifndef STINGRAYKIT_ALIGNED_STORAGE_H
#define STINGRAYKIT_ALIGNED_STORAGE_H


#include <cassert>
#include <stddef.h>

#include <stingraykit/Types.h>


namespace stingray
{

	namespace Detail
	{
		namespace EnlargementAlignmentMeasurer
		{
			template <typename T> struct alignment_of;

			template <typename T, int size_diff>
			struct AlignmentOfHelper
			{ static const size_t Value = size_diff; };

			template <typename T>
			struct AlignmentOfHelper<T, 0>
			{ static const size_t Value = alignment_of<T>::Value; };

			template <typename T>
			struct alignment_of
			{
				struct Enlarged { T x; char c; };

				static const size_t diff = sizeof(Enlarged) - sizeof(T);
				static const size_t Value = AlignmentOfHelper<Enlarged, diff>::Value;
			};
		}


		namespace SimpleAlignmentMeasurer
		{
			template <typename T>
			struct alignment_of
			{
				static const size_t size = sizeof(T);
				static const size_t Value = size ^ (size & (size - 1));
			};
		}


		namespace AlignedTypes
		{
#if defined(__GNUC__) || defined(__clang__)
			struct __attribute__((__aligned__(2))) a2 {};
			struct __attribute__((__aligned__(4))) a4 {};
			struct __attribute__((__aligned__(8))) a8 {};
			struct __attribute__((__aligned__(16))) a16 {};
			struct __attribute__((__aligned__(32))) a32 {};

			template <size_t N> struct type_with_alignment;
			template<> struct type_with_alignment<1>  { typedef char type; };
			template<> struct type_with_alignment<2>  { typedef AlignedTypes::a2 type; };
			template<> struct type_with_alignment<4>  { typedef AlignedTypes::a4 type; };
			template<> struct type_with_alignment<8>  { typedef AlignedTypes::a8 type; };
			template<> struct type_with_alignment<16> { typedef AlignedTypes::a16 type; };
			template<> struct type_with_alignment<32> { typedef AlignedTypes::a32 type; };

			typedef TypeList_6<
				integral_constant<size_t, 1>,
				integral_constant<size_t, 2>,
				integral_constant<size_t, 4>,
				integral_constant<size_t, 8>,
				integral_constant<size_t, 16>,
				integral_constant<size_t, 32> > SupportedAligners;

#else
			struct a2	{ u16 s; };
			struct a4	{ u32 s; };
			struct a8	{ double s; };
			struct a16	{ long double s; };

			template <size_t N> struct type_with_alignment;
			template<> struct type_with_alignment<1>	{ typedef char type; };
			template<> struct type_with_alignment<2>	{ typedef a2 type; };
			template<> struct type_with_alignment<4>	{ typedef a4 type; };
			template<> struct type_with_alignment<8>	{ typedef a8 type; };
			template<> struct type_with_alignment<16>	{ typedef a16 type; };

			typedef TypeList_5<
				integral_constant<size_t, 1>,
				integral_constant<size_t, 2>,
				integral_constant<size_t, 4>,
				integral_constant<size_t, 8>,
				integral_constant<size_t, 16> > SupportedAligners;
#endif
		}

		template<size_t Len, size_t Align>
		struct AlignedStorageImpl
		{
			union
#if defined(__GNUC__) || defined(__clang__)
			__attribute__((may_alias))
#endif
			data_t
			{
				char													_buf[Len];
				typename AlignedTypes::type_with_alignment<Align>::type	_align;
			} _data;
		};
	}


	template <typename T>
	struct alignment_of
	{
		static const size_t simple = Detail::SimpleAlignmentMeasurer::alignment_of<T>::Value;
		static const size_t enlarged = Detail::EnlargementAlignmentMeasurer::alignment_of<T>::Value;
		static const size_t Value = simple < enlarged ? simple : enlarged;

		typedef integral_constant<size_t, Value> ValueT;
	};


	template<typename T>
	struct SizeOf
	{
		typedef integral_constant<size_t, sizeof(T)> ValueT;
	};


	template<size_t Len, size_t Align>
	struct aligned_storage
	{
		typedef Detail::AlignedStorageImpl<Len, Align> type;
	};


// useful on gcc-4.4
#ifdef STINGRAY_USE_STRICT_ALIASING_STORAGE_FOR_WORKAROUND
	template<typename T, bool IsPod = IsBuiltinType<T>::Value || IsPointer<T>::Value>
	struct StorageFor
	{
		typename aligned_storage<sizeof(T), alignment_of<T>::Value>::type	_value;
		T*																	_ptr;

		void Ctor()											{ _ptr = new(&_value) T(); }

		template < typename P1 >
		void Ctor(const P1& p1)								{ _ptr = new(&_value) T(p1); }

		template < typename P1, typename P2 >
		void Ctor(const P1& p1, const P2& p2)				{ _ptr = new(&_value) T(p1, p2); }

		template < typename P1, typename P2, typename P3 >
		void Ctor(const P1& p1, const P2& p2, const P3& p3) { _ptr = new(&_value) T(p1, p2, p3); }

		void Dtor()
		{ Ref().~T(); }

		T& Ref()				{ return *_ptr; }
		const T& Ref() const	{ return *_ptr; }
	};

	template<typename T>
	struct StorageFor<T, true>
	{
		T	_value;

		StorageFor() : _value() { }

		void Ctor()		{ _value = T(); }
		void Ctor(T t)	{ _value = t; }
		void Dtor()		{ }

		T& Ref()				{ return _value; }
		const T& Ref() const	{ return _value; }
	};
#else

	template<typename T>
	struct StorageFor
	{
		typename aligned_storage<sizeof(T), alignment_of<T>::Value>::type	_value;

		void Ctor()											{ T* ptr = new(&_value) T(); (void)ptr; assert(ptr == &Ref()); }

		template < typename P1 >
		void Ctor(const P1& p1)								{ T* ptr = new(&_value) T(p1); (void)ptr; assert(ptr == &Ref()); }

		template < typename P1, typename P2 >
		void Ctor(const P1& p1, const P2& p2)				{ T* ptr = new(&_value) T(p1, p2); (void)ptr; assert(ptr == &Ref()); }

		template < typename P1, typename P2, typename P3 >
		void Ctor(const P1& p1, const P2& p2, const P3& p3)	{ T* ptr = new(&_value) T(p1, p2, p3); (void)ptr; assert(ptr == &Ref()); }

		void Dtor()
		{ Ref().~T(); }

		T& Ref()				{ return *static_cast<T*>(static_cast<void*>(&_value)); }
		const T& Ref() const	{ return *static_cast<const T*>(static_cast<const void*>(&_value)); }
	};
#endif


}


#endif
