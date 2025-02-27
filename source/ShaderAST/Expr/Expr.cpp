/*
See LICENSE file in root folder
*/
#include "ShaderAST/Expr/Expr.hpp"

#include "ShaderAST/Visitors/CloneExpr.hpp"

namespace ast::expr
{
	Expr::Expr( type::TypesCache & cache
		, type::TypePtr type
		, Kind kind
		, Flag flag )
		: m_cache{ &cache }
		, m_kind{ kind }
		, m_type{ std::move( type ) }
		, m_flags{ uint32_t( flag ) }
	{
	}

	ExprPtr Expr::clone()
	{
		return ExprCloner::submit( this );
	}
}
