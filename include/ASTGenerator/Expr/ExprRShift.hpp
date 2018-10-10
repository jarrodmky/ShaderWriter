/*
See LICENSE file in root folder
*/
#ifndef ___AST_ExprRShift_H___
#define ___AST_ExprRShift_H___
#pragma once

#include "ExprBinary.hpp"

namespace ast
{
	class ExprRShift
		: public ExprBinary
	{
	public:
		ExprRShift( TypePtr type
			, ExprPtr lhs
			, ExprPtr rhs );

		void accept( ExprVisitorPtr vis )override;
	};

	inline std::unique_ptr< ExprRShift > makeRShiftExpr( TypePtr type
		, ExprPtr lhs
		, ExprPtr rhs )
	{
		return std::make_unique< ExprRShift >( type
			, std::move( lhs )
			, std::move( rhs ) );
	}
}

#endif
