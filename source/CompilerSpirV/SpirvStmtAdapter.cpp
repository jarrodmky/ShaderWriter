/*
See LICENSE file in root folder
*/
#include "SpirvStmtAdapter.hpp"

#include "SpirvExprAdapter.hpp"
#include "SpirvExprEvaluator.hpp"

#include <ShaderAST/Shader.hpp>

#pragma warning( disable: 4706 )

namespace spirv
{
	ast::stmt::ContainerPtr StmtAdapter::submit( ast::stmt::Container * container
		, AdaptationData & adaptationData )
	{
		auto result = ast::stmt::makeContainer();
		StmtAdapter vis{ result, adaptationData };
		container->accept( &vis );
		return result;
	}

	StmtAdapter::StmtAdapter( ast::stmt::ContainerPtr & result
		, AdaptationData & adaptationData )
		: StmtCloner{ result }
		, m_adaptationData{ adaptationData }
	{
		auto cont = ast::stmt::makeContainer();
		m_ioDeclarations = cont.get();
		m_current->addStmt( std::move( cont ) );
	}

	ast::expr::ExprPtr StmtAdapter::doSubmit( ast::expr::Expr * expr )
	{
		return ExprAdapter::submit( expr, m_current, m_ioDeclarations, m_adaptationData );
	}

	void StmtAdapter::visitElseIfStmt( ast::stmt::ElseIf * stmt )
	{
		AST_Failure( "Unexpected ElseIf statement." );
	}

	void StmtAdapter::visitElseStmt( ast::stmt::Else * stmt )
	{
		AST_Failure( "Unexpected Else statement." );
	}

	void StmtAdapter::visitIfStmt( ast::stmt::If * stmt )
	{
		assert( stmt->getElseIfList().empty() && "ElseIf list is supposed to have been converted." );
		auto save = m_current;
		auto cont = ast::stmt::makeIf( doSubmit( stmt->getCtrlExpr() ) );
		m_current = cont.get();
		visitContainerStmt( stmt );
		m_current = save;

		auto currentIf = cont.get();
		m_current->addStmt( std::move( cont ) );

		if ( stmt->getElse() )
		{
			auto elseStmt = currentIf->createElse();
			m_current = elseStmt;
			visitContainerStmt( stmt->getElse() );
			m_current = save;
		}
	}

	void StmtAdapter::visitFunctionDeclStmt( ast::stmt::FunctionDecl * stmt )
	{
		if ( stmt->getFlags() )
		{
			if ( stmt->isEntryPoint() )
			{
				doProcessEntryPoint( stmt );
			}
			else if ( stmt->isPatchRoutine() )
			{
				doProcessPatchRoutine( stmt );
			}
		}
		else
		{
			ast::StmtCloner::visitFunctionDeclStmt( stmt );
		}
	}

	void StmtAdapter::visitInOutVariableDeclStmt( ast::stmt::InOutVariableDecl * stmt )
	{
	}

	void StmtAdapter::visitImageDeclStmt( ast::stmt::ImageDecl * stmt )
	{
		StmtCloner::visitImageDeclStmt( stmt );
	}

	void StmtAdapter::visitPerVertexDeclStmt( ast::stmt::PerVertexDecl * stmt )
	{
		StmtCloner::visitPerVertexDeclStmt( stmt );
	}

	void StmtAdapter::visitSampledImageDeclStmt( ast::stmt::SampledImageDecl * stmt )
	{
		StmtCloner::visitSampledImageDeclStmt( stmt );
	}

	void StmtAdapter::visitShaderStructBufferDeclStmt( ast::stmt::ShaderStructBufferDecl * stmt )
	{
		StmtCloner::visitShaderStructBufferDeclStmt( stmt );
	}

	void StmtAdapter::visitSimpleStmt( ast::stmt::Simple * stmt )
	{
		bool processed = false;

		if ( stmt->getExpr()->isConstant() )
		{
			if ( stmt->getExpr()->getKind() == ast::expr::Kind::eInit )
			{
				auto init = static_cast< ast::expr::Init * >( stmt->getExpr() );
				auto ident = init->getIdentifier();

				if ( ident )
				{
					m_adaptationData.context.constExprs.emplace( ident->getVariable()->getId()
						, doSubmit( init->getInitialiser() ) );
					processed = true;
				}
			}
			else if ( stmt->getExpr()->getKind() == ast::expr::Kind::eAggrInit )
			{
				auto aggrInit = static_cast< ast::expr::AggrInit * >( stmt->getExpr() );
				auto ident = aggrInit->getIdentifier();

				if ( ident )
				{
					ast::expr::ExprList initialisers;

					for ( auto & init : aggrInit->getInitialisers() )
					{
						initialisers.emplace_back( doSubmit( init.get() ) );
					}

					m_adaptationData.context.constAggrExprs.emplace( ident->getVariable()->getId()
						, std::move( initialisers ) );
					processed = true;
				}
			}
		}

		if ( !processed )
		{
			StmtCloner::visitSimpleStmt( stmt );
		}
	}

	void StmtAdapter::visitStructureDeclStmt( ast::stmt::StructureDecl * stmt )
	{
		StmtCloner::visitStructureDeclStmt( stmt );
	}

	void StmtAdapter::visitVariableDeclStmt( ast::stmt::VariableDecl * stmt )
	{
		StmtCloner::visitVariableDeclStmt( stmt );
	}

	void StmtAdapter::visitPreprocDefine( ast::stmt::PreprocDefine * preproc )
	{
		m_adaptationData.context.constExprs.emplace( preproc->getId()
			, doSubmit( preproc->getExpr() ) );
	}

	void StmtAdapter::visitPreprocElif( ast::stmt::PreprocElif * preproc )
	{
		visitContainerStmt( preproc );
	}

	void StmtAdapter::visitPreprocElse( ast::stmt::PreprocElse * preproc )
	{
		visitContainerStmt( preproc );
	}

	void StmtAdapter::visitPreprocEndif( ast::stmt::PreprocEndif * preproc )
	{
	}

	void StmtAdapter::visitPreprocIf( ast::stmt::PreprocIf * preproc )
	{
		bool isTrue = eval( preproc->getCtrlExpr(), m_adaptationData.context );

		if ( isTrue )
		{
			visitContainerStmt( preproc );
		}
		else
		{
			uint32_t i = 0u;

			while ( i < preproc->getElifList().size()
				&& !( isTrue = eval( static_cast< ast::stmt::PreprocElif const & >( *preproc->getElifList()[i] ).getCtrlExpr(), m_adaptationData.context ) ) )
			{
				++i;
			}

			if ( isTrue )
			{
				preproc->getElifList()[i]->accept( this );
			}
			else if ( preproc->getElse() )
			{
				preproc->getElse()->accept( this );
			}
		}
	}

	void StmtAdapter::visitPreprocIfDef( ast::stmt::PreprocIfDef * preproc )
	{
		bool isTrue = eval( preproc->getIdentExpr(), m_adaptationData.context );

		if ( isTrue )
		{
			visitContainerStmt( preproc );
		}
		else
		{
			uint32_t i = 0u;

			while ( i < preproc->getElifList().size()
				&& !( isTrue = eval( static_cast< ast::stmt::PreprocElif const & >( *preproc->getElifList()[i] ).getCtrlExpr(), m_adaptationData.context ) ) )
			{
				++i;
			}

			if ( isTrue )
			{
				preproc->getElifList()[i]->accept( this );
			}
			else if ( preproc->getElse() )
			{
				preproc->getElse()->accept( this );
			}
		}
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::ComputeInput const & compType )
	{
		auto type = compType.getType();
		m_current->addStmt( ast::stmt::makeInputComputeLayout( type
			, compType.getLocalSizeX()
			, compType.getLocalSizeY()
			, compType.getLocalSizeZ() ) );
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::FragmentInput const & fragType )
	{
		auto type = fragType.getType();
		m_current->addStmt( ast::stmt::makeFragmentLayout( type
			, fragType.getOrigin()
			, fragType.getCenter() ) );
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::GeometryOutput const & geomType )
	{
		auto type = geomType.getType();
		m_current->addStmt( ast::stmt::makeOutputGeometryLayout( type
			, geomType.getLayout()
			, geomType.getCount() ) );
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::GeometryInput const & geomType )
	{
		auto type = geomType.getType();
		m_current->addStmt( ast::stmt::makeInputGeometryLayout( type
			, geomType.getLayout() ) );
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::TessellationControlOutput const & tessType
		, bool isEntryPoint )
	{
		auto type = tessType.getType();
		m_current->addStmt( ast::stmt::makeOutputTessellationControlLayout( type
			, tessType.getDomain()
			, tessType.getPartitioning()
			, tessType.getTopology()
			, tessType.getOrder()
			, tessType.getOutputVertices() ) );
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::TessellationControlInput const & geomType
		, bool isEntryPoint )
	{
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::TessellationEvaluationInput const & tessType )
	{
		m_current->addStmt( ast::stmt::makeInputTessellationEvaluationLayout( tessType.getType()
			, tessType.getDomain()
			, tessType.getPartitioning()
			, tessType.getPrimitiveOrdering() ) );
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::TessellationInputPatch const & patchType )
	{
		var = m_adaptationData.config.getInputPatch( var );

		if ( !getStructType( var->getType() )->empty() )
		{
			m_current->addStmt( ast::stmt::makeInOutVariableDecl( var, patchType.getLocation() ) );
		}
	}

	void StmtAdapter::doProcess( ast::var::VariablePtr var
		, ast::type::TessellationOutputPatch const & patchType
		, bool isEntryPoint )
	{
		var = m_adaptationData.config.getOutputPatch( var );

		if ( !getStructType( var->getType() )->empty() )
		{
			m_current->addStmt( ast::stmt::makeInOutVariableDecl( var, patchType.getLocation() ) );
		}
	}

	void StmtAdapter::doProcessEntryPoint( ast::stmt::FunctionDecl * stmt )
	{
		auto & cache = stmt->getType()->getCache();
		auto funcType = cache.getFunction( cache.getVoid(), {} );
		doProcessInOut( stmt->getType(), true );
		auto save = m_current;
		auto cont = ast::stmt::makeFunctionDecl( funcType, stmt->getName(), stmt->getFlags() );
		m_current = cont.get();
		visitContainerStmt( stmt );

		for ( auto & pending : m_pending )
		{
			doProcessInOut( pending.second.funcType
				, false );
			visitContainerStmt( pending.second.statements.get() );
		}

		m_current = save;
		m_current->addStmt( std::move( cont ) );
	}

	void StmtAdapter::doProcessPatchRoutine( ast::stmt::FunctionDecl * stmt )
	{
		auto save = m_current;
		auto cont = ast::stmt::makeContainer();
		m_current = cont.get();
		visitContainerStmt( stmt );
		m_current = save;
		m_pending.emplace( stmt->getName()
			, PendingFunction{ stmt->getType(), std::move( cont ) } );
	}

	void StmtAdapter::doProcessInOut( ast::type::FunctionPtr funcType
		, bool isEntryPoint )
	{
		for ( auto & param : *funcType )
		{
			auto type = param->getType();

			switch ( type->getKind() )
			{
			case ast::type::Kind::eFragmentInput:
				doProcess( param, static_cast< ast::type::FragmentInput const & >( *type ) );
				break;
			case ast::type::Kind::eGeometryOutput:
				doProcess( param, static_cast< ast::type::GeometryOutput const & >( *type ) );
				break;
			case ast::type::Kind::eGeometryInput:
				doProcess( param, static_cast< ast::type::GeometryInput const & >( *type ) );
				break;
			case ast::type::Kind::eTessellationInputPatch:
				doProcess( param, static_cast< ast::type::TessellationInputPatch const & >( *type ) );
				break;
			case ast::type::Kind::eTessellationControlInput:
				doProcess( param, static_cast< ast::type::TessellationControlInput const & >( *type ), isEntryPoint );
				break;
			case ast::type::Kind::eTessellationEvaluationInput:
				doProcess( param, static_cast< ast::type::TessellationEvaluationInput const & >( *type ) );
				break;
			case ast::type::Kind::eComputeInput:
				doProcess( param, static_cast< ast::type::ComputeInput const & >( *type ) );
				break;
			case ast::type::Kind::eTessellationOutputPatch:
				doProcess( param, static_cast< ast::type::TessellationOutputPatch const & >( *type ), isEntryPoint );
				break;
			case ast::type::Kind::eTessellationControlOutput:
				doProcess( param, static_cast< ast::type::TessellationControlOutput const & >( *type ), isEntryPoint );
				break;
			default:
				break;
			}
		}
	}

	void StmtAdapter::doDeclareStruct( ast::type::StructPtr const & structType )
	{
		if ( m_declaredStructs.emplace( structType ).second )
		{
			m_current->addStmt( ast::stmt::makeStructureDecl( structType ) );
		}
	}
}
