/*
See LICENSE file in root folder
*/
#ifndef ___SDW_Mat2x4_H___
#define ___SDW_Mat2x4_H___
#pragma once

#include "ShaderWriter/VecTypes/Vec4.hpp"

namespace sdw
{
	template< typename ValueT >
	struct Mat2x4T
		: public Value
	{
		SDW_DeclValue( , Mat2x4T );

		using ValueType = ValueT;
		using my_vec = Vec4T< ValueT >;
		using my_mat = Mat2x4T< ValueT >;

		inline Mat2x4T( ShaderWriter & writer
			, expr::ExprPtr expr
			, bool enabled );

		template< typename RhsT >
		inline Mat2x4T< ValueT > & operator=( RhsT const & rhs );
		template< typename IndexT >
		inline Vec4T< ValueT > operator[]( IndexT const & rhs )const;
		inline Vec4T< ValueT > operator[]( int32_t offset )const;
		inline Vec4T< ValueT > operator[]( uint32_t offset )const;

		static inline ast::type::TypePtr makeType( ast::type::TypesCache & cache );
	};
}

#include "Mat2x4.inl"

#endif
