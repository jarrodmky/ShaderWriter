/*
See LICENSE file in root folder
*/
#include "ShaderAST/Expr/ExprStreamAppend.hpp"

#include "ShaderAST/Expr/ExprVisitor.hpp"

namespace ast::expr
{
	StreamAppend::StreamAppend( type::TypePtr type
		, ExprPtr operand )
		: Unary{ std::move( type )
			, std::move( operand )
			, Kind::eStreamAppend }
	{
	}

	void StreamAppend::accept( VisitorPtr vis )
	{
		vis->visitStreamAppendExpr( this );
	}
}
