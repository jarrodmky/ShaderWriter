/*
See LICENSE file in root folder
*/
#include "HlslStmtAdapter.hpp"

#include "HlslExprAdapter.hpp"
#include "HlslHelpers.hpp"
#include "HlslImageAccessFunctions.hpp"
#include "HlslIntrinsicFunctions.hpp"
#include "HlslTextureAccessFunctions.hpp"

#include <ShaderAST/Shader.hpp>

#include <ShaderAST/Type/TypeSampledImage.hpp>
#include <ShaderAST/Type/TypeImage.hpp>

namespace hlsl
{
	ast::stmt::ContainerPtr StmtAdapter::submit( HlslShader & shader
		, ast::stmt::Container * container
		, IntrinsicsConfig const & intrinsicsConfig
		, HlslConfig const & writerConfig
		, AdaptationData & adaptationData )
	{
		auto result = ast::stmt::makeContainer();
		StmtAdapter vis{ shader, intrinsicsConfig, writerConfig, adaptationData, result };
		container->accept( &vis );
		return result;
	}

	StmtAdapter::StmtAdapter( HlslShader & shader
		, IntrinsicsConfig const & intrinsicsConfig
		, HlslConfig const & writerConfig
		, AdaptationData & adaptationData
		, ast::stmt::ContainerPtr & result )
		: StmtCloner{ result }
		, m_intrinsicsConfig{ intrinsicsConfig }
		, m_writerConfig{ writerConfig }
		, m_adaptationData{ adaptationData }
		, m_shader{ shader }
		, m_cache{ shader.getTypesCache() }
	{
		auto cont = ast::stmt::makeContainer();
		compileHlslIntrinsicFunctions( cont.get(), m_intrinsicsConfig );
		compileHlslTextureAccessFunctions( cont.get(), m_intrinsicsConfig );
		compileHlslImageAccessFunctions( cont.get(), m_intrinsicsConfig );
		m_intrinsics = cont.get();
		m_current->addStmt( std::move( cont ) );

		cont = ast::stmt::makeContainer();
		m_inOutDeclarations = cont.get();
		m_current->addStmt( std::move( cont ) );
	}

	ast::expr::ExprPtr StmtAdapter::doSubmit( ast::expr::Expr * expr )
	{
		return ExprAdapter::submit( m_cache
			, expr
			, m_current
			, m_intrinsicsConfig
			, m_writerConfig
			, m_adaptationData
			, m_intrinsics );
	}

	void StmtAdapter::linkVars( ast::var::VariablePtr textureSampler
		, ast::var::VariablePtr texture
		, ast::var::VariablePtr sampler )
	{
		m_adaptationData.linkedVars.emplace( std::move( textureSampler )
			, std::make_pair( std::move( texture )
				, std::move( sampler ) ) );
	}

	void StmtAdapter::visitFunctionDeclStmt( ast::stmt::FunctionDecl * stmt )
	{
		if ( stmt->getFlags() )
		{
			m_adaptationData.updateCurrentEntryPoint( stmt );

			auto & cache = stmt->getType()->getCache();
			// Write function content into a temporary container, registering I/O.
			auto cont = ast::stmt::makeContainer();
			auto save = m_current;
			m_current = cont.get();
			visitContainerStmt( stmt );
			m_current = save;


			if ( m_adaptationData.needsSeparateFunc() )
			{
				// Write SDW_main function
				m_inOutDeclarations->addStmt( m_adaptationData.writeGlobals( m_declaredStructs ) );
				auto sdwMainCont = ast::stmt::makeFunctionDecl( cache.getFunction( stmt->getType()->getReturnType()
					, ast::var::VariableList{} )
					, "SDW_" + stmt->getName() );
				sdwMainCont->addStmt( std::move( cont ) );
				m_current->addStmt( std::move( sdwMainCont ) );

				// Write main function
				writeMain( stmt );
			}
			else
			{
				// Write main function, with only used parameters.
				m_inOutDeclarations->addStmt( m_adaptationData.writeGlobals( m_declaredStructs ) );
				ast::var::VariableList parameters;
				auto retType = m_adaptationData.fillParameters( parameters, *m_current );
				auto mainCont = ast::stmt::makeFunctionDecl( cache.getFunction( ( retType ? retType : stmt->getType()->getReturnType() )
					, parameters )
					, stmt->getName()
					, stmt->getFlags() );
				mainCont->addStmt( m_adaptationData.writeLocalesBegin() );
				mainCont->addStmt( std::move( cont ) );
				mainCont->addStmt( m_adaptationData.writeLocalesEnd() );
				m_current->addStmt( std::move( mainCont ) );
			}

			m_adaptationData.updateCurrentEntryPoint( nullptr );
		}
		else
		{
			auto save = m_current;
			auto cont = rewriteFuncHeader( stmt );
			m_current = cont.get();
			visitContainerStmt( stmt );
			m_current = save;
			m_current->addStmt( std::move( cont ) );
		}
	}

	void StmtAdapter::visitInOutVariableDeclStmt( ast::stmt::InOutVariableDecl * stmt )
	{
	}

	void StmtAdapter::visitInputComputeLayoutStmt( ast::stmt::InputComputeLayout * stmt )
	{
	}

	void StmtAdapter::visitInputGeometryLayoutStmt( ast::stmt::InputGeometryLayout * stmt )
	{
	}

	void StmtAdapter::visitPerVertexDeclStmt( ast::stmt::PerVertexDecl * stmt )
	{
	}

	void StmtAdapter::visitSampledImageDeclStmt( ast::stmt::SampledImageDecl * stmt )
	{
		auto originalVar = stmt->getVariable();
		ast::type::TypePtr sampledType;
		ast::type::TypePtr samplerType;
		ast::type::TypePtr imageType;
		ast::type::ImageConfiguration config;

		if ( originalVar->getType()->getKind() == ast::type::Kind::eArray )
		{
			auto arrayType = std::static_pointer_cast< ast::type::Array >( originalVar->getType() );
			auto realSampledType = std::static_pointer_cast< ast::type::SampledImage >( arrayType->getType() );
			imageType = m_cache.getArray( realSampledType->getImageType(), arrayType->getArraySize() );
			sampledType = m_cache.getArray( realSampledType, arrayType->getArraySize() );
			config = realSampledType->getConfig();

			if ( !m_intrinsicsConfig.requiresShadowSampler )
			{
				samplerType = m_cache.getArray( m_cache.getSampler( false ), arrayType->getArraySize() );
			}
			else
			{
				samplerType = m_cache.getArray( realSampledType->getSamplerType(), arrayType->getArraySize() );
			}
		}
		else
		{
			auto realSampledType = std::static_pointer_cast< ast::type::SampledImage >( originalVar->getType() );
			imageType = realSampledType->getImageType();
			sampledType = realSampledType;
			config = realSampledType->getConfig();

			if ( !m_intrinsicsConfig.requiresShadowSampler )
			{
				samplerType = m_cache.getSampler( false );
			}
			else
			{
				samplerType = realSampledType->getSamplerType();
			}
		}

		if ( config.dimension == ast::type::ImageDim::eBuffer )
		{
			// Create Image
			auto textureVar = m_shader.registerImage( originalVar->getName()
				, imageType
				, stmt->getBindingPoint()
				, stmt->getDescriptorSet() );
			textureVar->updateFlag( ast::var::Flag::eImplicit );
			m_current->addStmt( ast::stmt::makeImageDecl( textureVar
				, stmt->getBindingPoint()
				, stmt->getDescriptorSet() ) );
		}
		else
		{
			// Create Image
			auto textureVar = m_shader.registerImage( originalVar->getName() + "_texture"
				, imageType
				, stmt->getBindingPoint()
				, stmt->getDescriptorSet() );
			textureVar->updateFlag( ast::var::Flag::eImplicit );
			m_current->addStmt( ast::stmt::makeImageDecl( textureVar
				, stmt->getBindingPoint()
				, stmt->getDescriptorSet() ) );

			// Create Sampler
			auto samplerVar = m_shader.registerSampler( originalVar->getName() + "_sampler"
				, samplerType
				, stmt->getBindingPoint()
				, stmt->getDescriptorSet() );
			samplerVar->updateFlag( ast::var::Flag::eImplicit );
			m_current->addStmt( ast::stmt::makeSamplerDecl( samplerVar
				, stmt->getBindingPoint()
				, stmt->getDescriptorSet() ) );

			// Link them
			linkVars( originalVar, textureVar, samplerVar );
		}
	}

	void StmtAdapter::visitShaderBufferDeclStmt( ast::stmt::ShaderBufferDecl * stmt )
	{
		auto ssboVar = stmt->getVariable();
		m_adaptationData.ssboList.push_back( ssboVar );
		m_current->addStmt( ast::stmt::makeStructureDecl( stmt->getType() ) );
		m_current->addStmt( ast::stmt::makeShaderStructBufferDecl( stmt->getSsboName()
			, ast::var::makeVariable( ++m_adaptationData.nextVarId
				, ssboVar->getType()
				, ssboVar->getName() + "Inst" )
			, ssboVar
			, stmt->getBindingPoint()
			, stmt->getDescriptorSet() ) );
		uint32_t mbrIndex = 0u;

		for ( auto & curStmt : *stmt )
		{
			assert( curStmt->getKind() == ast::stmt::Kind::eVariableDecl );
			auto var = static_cast< ast::stmt::VariableDecl const & >( *curStmt ).getVariable();
			m_adaptationData.replacedVars.emplace( var
				, ast::expr::makeMbrSelect( ast::expr::makeArrayAccess( ssboVar->getType()
						, ast::expr::makeIdentifier( m_cache
							, ast::var::makeVariable( ++m_adaptationData.nextVarId
								, m_cache.getArray( ssboVar->getType(), 1u )
								, ssboVar->getName() ) )
						, ast::expr::makeLiteral( m_cache, 0 ) )
					, mbrIndex++
					, uint32_t( ast::var::Flag::eUniform ) ) );
		}
	}

	void StmtAdapter::visitShaderStructBufferDeclStmt( ast::stmt::ShaderStructBufferDecl * stmt )
	{
		m_adaptationData.ssboList.push_back( stmt->getSsboInstance() );
		m_current->addStmt( ast::stmt::makeShaderStructBufferDecl( stmt->getSsboName()
			, stmt->getSsboInstance()
			, stmt->getData()
			, stmt->getBindingPoint()
			, stmt->getDescriptorSet() ) );
	}

	void StmtAdapter::visitVariableDeclStmt( ast::stmt::VariableDecl * stmt )
	{
		m_current->addStmt( ast::stmt::makeVariableDecl( stmt->getVariable() ) );
	}

	void StmtAdapter::visitOutputGeometryLayoutStmt( ast::stmt::OutputGeometryLayout * stmt )
	{
	}

	void StmtAdapter::visitPreprocExtension( ast::stmt::PreprocExtension * preproc )
	{
	}

	void StmtAdapter::visitPreprocVersion( ast::stmt::PreprocVersion * preproc )
	{
	}

	void StmtAdapter::writeMain( ast::stmt::FunctionDecl * stmt )
	{
		assert( stmt->getType()->getReturnType()->getKind() == ast::type::Kind::eVoid );
		ast::var::VariableList mainParameters;
		ast::type::TypePtr mainRetType = m_cache.getVoid();
		auto retType = m_adaptationData.fillParameters( mainParameters, *m_current );

		if ( retType )
		{
			mainRetType = retType;
		}

		auto cont = ast::stmt::makeFunctionDecl( m_cache.getFunction( mainRetType, mainParameters )
			, stmt->getName()
			, stmt->getFlags() );
		cont->addStmt( m_adaptationData.writeLocalesBegin() );

		// Call SDW_main function.
		cont->addStmt( ast::stmt::makeSimple( ast::expr::makeFnCall( m_cache.getVoid()
			, ast::expr::makeIdentifier( m_cache
				, ast::var::makeFunction( ++m_adaptationData.nextVarId
					, m_cache.getFunction( m_cache.getVoid(), ast::var::VariableList{} )
					, "SDW_" + stmt->getName() ) )
			, ast::expr::ExprList{} ) ) );

		cont->addStmt( m_adaptationData.writeLocalesEnd() );

		m_current->addStmt( std::move( cont ) );
	}

	ast::stmt::FunctionDeclPtr StmtAdapter::rewriteFuncHeader( ast::stmt::FunctionDecl * stmt )
	{
		ast::var::VariableList params;
		// Split sampled textures in sampler + texture in parameters list.
		for ( auto & param : *stmt->getType() )
		{
			auto it = updateLinkedVars( param
				, m_adaptationData.linkedVars
				, m_adaptationData.nextVarId );

			if ( it != m_adaptationData.linkedVars.end() )
			{
				params.push_back( it->second.first );
				params.push_back( it->second.second );
			}
			else
			{
				params.push_back( param );
			}
		}

		return ast::stmt::makeFunctionDecl( m_cache.getFunction( stmt->getType()->getReturnType(), params )
			, stmt->getName()
			, stmt->getFlags() );
	}
}
