/*
See LICENSE file in root folder
*/
#ifndef ___SDW_GlslExprAdapter_H___
#define ___SDW_GlslExprAdapter_H___
#pragma once

#include "CompilerGlsl/GlslStmtAdapter.hpp"

#include <ShaderAST/Visitors/CloneExpr.hpp>

namespace glsl
{
	class ExprAdapter
		: public ast::ExprCloner
	{
	public:
		static ast::expr::ExprPtr submit( ast::type::TypesCache & cache
			, ast::expr::Expr * expr
			, AdaptationData & adaptationData );
		static ast::expr::ExprPtr submit( ast::type::TypesCache & cache
			, ast::expr::ExprPtr const & expr
			, AdaptationData & adaptationData );

	private:
		ExprAdapter( ast::type::TypesCache & cache
			, AdaptationData & adaptationData
			, ast::expr::ExprPtr & result );

		ast::expr::ExprPtr doSubmit( ast::expr::Expr * expr )override;
		void visitImageAccessCallExpr( ast::expr::ImageAccessCall * expr )override;
		void visitIntrinsicCallExpr( ast::expr::IntrinsicCall * expr )override;
		void visitMbrSelectExpr( ast::expr::MbrSelect * expr )override;
		void visitStreamAppendExpr( ast::expr::StreamAppend * expr )override;
		void visitTextureAccessCallExpr( ast::expr::TextureAccessCall * expr )override;

		void doProcessImageStore( ast::expr::ImageAccessCall * expr );
		void doProcessTextureShadow( ast::expr::TextureAccessCall * expr );
		void doProcessTextureSample( ast::expr::TextureAccessCall * expr );
		void doProcessTextureGather( ast::expr::TextureAccessCall * expr );

		ast::expr::ExprPtr doProcessIOMbr( ast::expr::Expr * outer
			, uint32_t mbrIndex
			, uint32_t mbrFlags
			, bool isInput
			, IOVars & io );
		ast::expr::ExprPtr doProcessMbr( ast::expr::Expr * outer
			, uint32_t mbrIndex
			, uint32_t mbrFlags );

	private:
		ast::type::TypesCache & m_cache;
		AdaptationData & m_adaptationData;
	};
}

#endif
