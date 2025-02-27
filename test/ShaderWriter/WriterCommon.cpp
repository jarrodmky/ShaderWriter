#include "WriterCommon.hpp"

#include "CompileGLSL.hpp"
#include "CompileHLSL.hpp"
#include "CompileSPIRV.hpp"
#include "GlslToSpv.hpp"

#if SDW_HasCompilerGlsl
#	include <CompilerGlsl/compileGlsl.hpp>
#endif
#if SDW_HasCompilerHlsl
#	include <CompilerHlsl/compileHlsl.hpp>
#endif
#if SDW_HasCompilerSpirV
#	include <CompilerSpirV/compileSpirV.hpp>
#	if SDW_Test_HasVulkan && SDW_HasVulkanLayer
#		include <VulkanLayer/PipelineBuilder.hpp>
#		include <VulkanLayer/ProgramPipeline.hpp>
#	endif
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wtype-limits"
#include "spirv_cpp.hpp"
#include "spirv_cross_util.hpp"
#include "spirv_glsl.hpp"
#include "spirv_hlsl.hpp"
#pragma GCC diagnostic pop

namespace test
{
	namespace
	{
#if SDW_HasCompilerGlsl

		glsl::GlslConfig const & getDefaultGlslConfig()
		{
			static glsl::GlslConfig const result
			{
				ast::ShaderStage::eCompute, // shaderStage;
				460, // shaderLanguageVersion{ 430 };
				false, // vulkanGlsl{ false };
				false, // flipVertY{ false };
				false, // fixupClipDepth{ false };
				true, // hasStd430Layout{ false };
				true, // hasShaderStorageBuffers{ false };
				true, // hasDescriptorSets{ false };
				true, // hasBaseInstance{ false };
			};
			return result;
		}

#endif

		spv::ExecutionModel getExecutionModel( ast::ShaderStage stage )
		{
			spv::ExecutionModel result{};

			switch ( stage )
			{
			case ast::ShaderStage::eVertex:
				result = spv::ExecutionModelVertex;
				break;
			case ast::ShaderStage::eGeometry:
				result = spv::ExecutionModelGeometry;
				break;
			case ast::ShaderStage::eTessellationControl:
				result = spv::ExecutionModelTessellationControl;
				break;
			case ast::ShaderStage::eTessellationEvaluation:
				result = spv::ExecutionModelTessellationEvaluation;
				break;
			case ast::ShaderStage::eFragment:
				result = spv::ExecutionModelFragment;
				break;
			case ast::ShaderStage::eCompute:
				result = spv::ExecutionModelGLCompute;
				break;
			default:
				AST_Failure( "Unsupported shader stage flag" );
				break;
			}

			return result;
		}

		void doSetEntryPoint( ast::ShaderStage stage
			, spirv_cross::CompilerGLSL & compiler )
		{
			auto model = getExecutionModel( stage );
			std::string entryPoint;

			for ( auto & e : compiler.get_entry_points_and_stages() )
			{
				if ( entryPoint.empty() && e.execution_model == model )
				{
					entryPoint = e.name;
				}
			}

			if ( !entryPoint.empty() )
			{
				compiler.set_entry_point( entryPoint, model );
			}
		}

		void doSetupOptions( spirv_cross::CompilerGLSL & compiler )
		{
			auto options = compiler.get_common_options();
			options.separate_shader_objects = true;
			options.vulkan_semantics = false;
			compiler.set_common_options( options );
		}

		void doSetupHlslOptions( spirv_cross::CompilerHLSL & compiler )
		{
			auto options = compiler.get_hlsl_options();
			options.shader_model = 50;
			compiler.set_hlsl_options( options );
		}

		std::string compileSpirV( std::string const & language
			, spirv_cross::Compiler & compiler
			, test::TestCounts & testCounts )
		{
			std::string result;
			try
			{
				result = compiler.compile();
			}
			catch ( spirv_cross::CompilerError & exc )
			{
				testCounts << "SPIRV-Cross(" << language << "), shader compilation failed: " << exc.what() << endl;
				throw;
			}
			catch ( std::exception & exc )
			{
				testCounts << "SPIRV-Cross(" << language << "), shader compilation failed: " << exc.what() << endl;
				throw;
			}
			catch ( ... )
			{
				testCounts << "SPIRV-Cross(" << language << "), shader compilation failed: Unknown error" << endl;
				throw;
			}

			return result;
		}

		std::string validateSpirVToGlsl( std::vector< uint32_t > const & spirv
			, ast::ShaderStage stage
			, test::TestCounts & testCounts )
		{
			auto compiler = std::make_unique< spirv_cross::CompilerGLSL >( spirv );
			doSetEntryPoint( stage, *compiler );
			doSetupOptions( *compiler );
			return compileSpirV( "GLSL", *compiler, testCounts );
		}

		std::string validateSpirVToHlsl( std::vector< uint32_t > const & spirv
			, ast::ShaderStage stage
			, test::TestCounts & testCounts )
		{
			auto compiler = std::make_unique< spirv_cross::CompilerHLSL >( spirv );
			doSetEntryPoint( stage, *compiler );
			doSetupHlslOptions( *compiler );
			doSetupOptions( *compiler );
			return compileSpirV( "HLSL", *compiler, testCounts );
		}

		void displayShader( std::string const & name
			, std::string const & shader
			, test::TestCounts & testCounts
			, bool force )
		{
			if ( force )
			{
				testCounts << "////////////////////////////////////////////////////////////" << endl;
				testCounts << "// " << name << endl;
				testCounts << "////////////////////////////////////////////////////////////" << endl;
				testCounts << endl << shader << endl;
			}
		}

		bool validateSpirV( ::ast::Shader const & shader
			, std::vector< uint32_t > spirv
			, std::string const & text
			, sdw_test::TestCounts & testCounts
			, uint32_t infoIndex
			, bool checkRef )
		{
			//auto parsedShader = spirv::parseSpirv( shader.getType(), spirv );
			std::string errors;
			auto result = test::compileSpirV( shader, spirv, errors, testCounts, infoIndex );
			check( errors.empty() || !checkRef );

			if ( !errors.empty() )
			{
				testCounts << "VkShaderModule creation raised messages, for CompilerSpv output:" << endl;
				testCounts << errors << endl;
				result = false;

				if ( checkRef )
				{
					auto fileName = getExecutableDirectory() + testCounts.testName + std::to_string( testCounts.getTotalCount() ) + ".spv";
					FILE * fileOut = fopen( fileName.c_str(), "wb" );

					if ( fileOut )
					{
						fwrite( spirv.data()
							, sizeof( uint32_t )
							, spirv.size()
							, fileOut );
						fclose( fileOut );
					}

					fileName = getExecutableDirectory() + testCounts.testName + std::to_string( testCounts.getTotalCount() ) + ".ref.spv";
					FILE * fileIn = fopen( fileName.c_str(), "rb" );

					if ( fileIn )
					{
						fseek( fileIn, 0, SEEK_END );
						auto size = ftell( fileIn );
						fseek( fileIn, 0, SEEK_SET );

						if ( size > 0 && ( size % sizeof( uint32_t ) ) == 0 )
						{
							spirv.resize( size / sizeof( uint32_t ) );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
							(void)fread( spirv.data()
								, sizeof( uint32_t )
								, spirv.size()
								, fileIn );
#pragma GCC diagnostic pop
						}

						fclose( fileIn );
						validateSpirV( shader, spirv, errors, testCounts, infoIndex, false );
					}
				}
			}

			return result;
		}

		void validateSpirV( ::ast::Shader const & shader
			, std::vector< uint32_t > const & spirv
			, std::string const & text
			, ::sdw::SpecialisationInfo const & specialisation
			, Compilers const & compilers
			, sdw_test::TestCounts & testCounts
			, uint32_t infoIndex )
		{
			auto isValidated = validateSpirV( shader, spirv, text, testCounts, infoIndex, true );
			check( isValidated );

			if ( !isValidated )
			{
				displayShader( "SPIR-V", text, testCounts, true );

#if SDW_HasCompilerGlsl

				try
				{
					auto glslangSpirv = compileGlslToSpv( shader.getType()
						, glsl::compileGlsl( shader
							, specialisation
							, getDefaultGlslConfig() ) );
					std::string errors;
					test::compileSpirV( shader, glslangSpirv, errors, testCounts, infoIndex );

					if ( !errors.empty() )
					{
						testCounts << "VkShaderModule creation raised messages, for glslang output:" << endl;
						testCounts << errors << endl;
					}
				}
				catch ( std::exception & exc )
				{
					testCounts << exc.what() << endl;
					throw;
				}

#endif
			}

			if ( compilers.glsl )
			{
				auto crossGlsl = test::validateSpirVToGlsl( spirv, shader.getType(), testCounts );
				displayShader( "SPIRV-Cross GLSL", crossGlsl, testCounts, compilers.forceDisplay );
			}

			if ( compilers.hlsl )
			{
				auto crossHlsl = test::validateSpirVToHlsl( spirv, shader.getType(), testCounts );
				displayShader( "SPIRV-Cross HLSL", crossHlsl, testCounts, compilers.forceDisplay );
			}
		}

		void testWriteDebug( ::ast::Shader const & shader
			, ::sdw::SpecialisationInfo const & specialisation
			, Compilers const & compilers
			, test::TestCounts & testCounts )
		{
			auto debug = ::sdw::writeDebug( shader );
			displayShader( "Statements", debug, testCounts, compilers.forceDisplay );
		}

		void testWriteGlsl( ::ast::Shader const & shader
			, ::sdw::SpecialisationInfo const & specialisation
			, Compilers const & compilers
			, sdw_test::TestCounts & testCounts )
		{
#if SDW_HasCompilerGlsl

			if ( compilers.glsl )
			{
				auto validate = [&]()
				{
					std::string errors;
					auto glsl = glsl::compileGlsl( shader
						, specialisation
						, getDefaultGlslConfig() );
					auto isCompiled = compileGlsl( glsl
						, shader.getType()
						, errors
						, testCounts );
					check( isCompiled );

					if ( !isCompiled )
					{
						displayShader( "GLSL", glsl, testCounts, true );
						testCounts << errors << endl;
					}
					else
					{
						displayShader( "GLSL", glsl, testCounts, compilers.forceDisplay );
					}
				};
				checkNoThrow( validate() );
			}

#endif
		}

		void testWriteHlsl( ::ast::Shader const & shader
			, ::sdw::SpecialisationInfo const & specialisation
			, Compilers const & compilers
			, sdw_test::TestCounts & testCounts )
		{
#if SDW_HasCompilerHlsl

			if ( compilers.hlsl )
			{
				auto validate = [&]()
				{
					std::string errors;
					auto hlsl = hlsl::compileHlsl( shader
						, specialisation
						, hlsl::HlslConfig{} );
					bool isCompiled = compileHlsl( hlsl
						, shader.getType()
						, errors
						, testCounts );
					check( isCompiled );

					if ( !isCompiled )
					{
						displayShader( "HLSL", hlsl, testCounts, true );
						testCounts << errors << endl;
					}
					else
					{
						displayShader( "HLSL", hlsl, testCounts, compilers.forceDisplay );
					}
				};
				checkNoThrow( validate() );
			}

#endif
		}

		uint32_t getVkMajor( uint32_t vkVersion )
		{
			return ( ( vkVersion >> 22u ) & 0x7FU );
		}

		uint32_t getVkMinor( uint32_t vkVersion )
		{
			return ( ( vkVersion >> 12u ) & 0x3FFU );
		}

		std::string printVkVersion( uint32_t vkVersion )
		{
			std::stringstream stream;
			stream << getVkMajor( vkVersion ) << "." << getVkMinor( vkVersion );
			return stream.str();
		}

		uint32_t getSpvMajor( uint32_t spvVersion )
		{
			return ( spvVersion >> 16u );
		}

		uint32_t getSpvMinor( uint32_t spvVersion )
		{
			return ( ( spvVersion >> 8u ) & 0xFF );
		}

		std::string printSpvVersion( uint32_t spvVersion )
		{
			std::stringstream stream;
			stream << getSpvMajor( spvVersion ) << "." << getSpvMinor( spvVersion );
			return stream.str();
		}

		void testWriteSpirVOnIndex( ::ast::Shader const & shader
			, ::sdw::SpecialisationInfo const & specialisation
			, Compilers const & compilers
			, sdw_test::TestCounts & testCounts
			, uint32_t infoIndex )
		{
#if SDW_HasCompilerSpirV

			if ( compilers.spirV
				&& testCounts.isInitialised( infoIndex ) )
			{
				auto validate = [&]( bool availableExtensions )
				{
					try
					{
						spirv::SpirVExtensionSet extensions;
						spirv::SpirVConfig config{};
						config.specVersion = testCounts.getSpirVVersion( infoIndex );

						if ( availableExtensions )
						{
							config.availableExtensions = &extensions;
						}

						auto textSpirv = spirv::writeSpirv( shader, config );

						if ( textSpirv.empty() )
						{
							testCounts << "Empty shader" << endl;
							return;
						}

						displayShader( "SPIR-V", textSpirv, testCounts, compilers.forceDisplay );
						std::vector< uint32_t > spirv;

						try
						{
							spirv = spirv::serialiseSpirv( shader, config );
							success();
						}
						catch ( ... )
						{
							failure( "testWriteSpirV" );
							displayShader( "SPIR-V", textSpirv, testCounts, true );
							throw;
						}

						try
						{
							test::validateSpirV( shader
								, spirv
								, textSpirv
								, specialisation
								, compilers
								, testCounts
								, infoIndex );
							success();
						}
						catch ( spirv_cross::CompilerError & exc )
						{
							std::string text = exc.what();

							if ( text.find( "not supported in HLSL" ) == std::string::npos
								&& text.find( "not supported on HLSL" ) == std::string::npos
								&& text.find( "exist in HLSL" ) == std::string::npos
								&& text.find( "Unsupported builtin in HLSL" ) == std::string::npos
								&& text.find( "Unsupported execution model" ) == std::string::npos
								&& text.find( "No function currently in scope" ) == std::string::npos
								&& text.find( "Cannot subdivide a scalar value!" ) == std::string::npos
								&& text.find( "NumWorkgroups builtin is used" ) == std::string::npos
								&& text.find( "Cannot resolve expression type" ) == std::string::npos )
							{
								failure( "testWriteSpirV" );
								displayShader( "SPIR-V", textSpirv, testCounts, true );
								testCounts << "spirv_cross exception: " << text << endl;
								throw;
							}
						}
						catch ( std::exception & exc )
						{
							failure( "testWriteSpirV" );
							displayShader( "SPIR-V", textSpirv, testCounts, true );
							testCounts << "std exception: " << exc.what() << endl;
							throw;
						}
					}
					catch ( ... )
					{
					}
				};
				testCounts.incIndent();
				testCounts << "Vulkan " << printVkVersion( testCounts.getVulkanVersion( infoIndex ) )
					<< " - SPIR-V " << printSpvVersion( testCounts.getSpirVVersion( infoIndex ) ) << endl;
				checkNoThrow( validate( false ) );
				checkNoThrow( validate( true ) );
				testCounts.decIndent();
			}

#endif
		}

		std::string printStage( ast::ShaderStage stage )
		{
			switch ( stage )
			{
			case ast::ShaderStage::eVertex:
				return "Vertex";
			case ast::ShaderStage::eTessellationControl:
				return "Tessellation Control";
			case ast::ShaderStage::eTessellationEvaluation:
				return "Tessellation Evaluation";
			case ast::ShaderStage::eGeometry:
				return "Geometry";
			case ast::ShaderStage::eFragment:
				return "Fragment";
			case ast::ShaderStage::eCompute:
				return "Compute";
			default:
				return "Unknown???"; 
			}
		}

		void testWriteSpirV( ::ast::Shader const & shader
			, ::sdw::SpecialisationInfo const & specialisation
			, Compilers const & compilers
			, sdw_test::TestCounts & testCounts )
		{
			for ( uint32_t infoIndex = 0u; infoIndex < testCounts.getSpirvInfosSize(); ++infoIndex )
			{
				testWriteSpirVOnIndex( shader
					, specialisation
					, compilers
					, testCounts
					, infoIndex );
			}
		}

		std::vector< uint8_t > getSpecData( ::sdw::SpecConstantInfo const & info )
		{
			return std::vector< uint8_t >( size_t( getSize( info.type
					, ast::type::MemoryLayout::eStd430 ) )
				, 0 );
		}

		::sdw::SpecialisationInfo getSpecialisationInfo( ::ast::Shader const & shader )
		{
			auto & specInfo = shader.getSpecConstants();
			::sdw::SpecialisationInfo result;

			for ( auto & info : specInfo )
			{
				result.data.push_back( { info.second, getSpecData( info.second ) } );
			}

			return result;
		}

		void validateShadersOnIndex( ast::ShaderArray const & shaders
			, sdw_test::TestCounts & testCounts
			, uint32_t infoIndex
			, Compilers const & compilers )
		{
#if SDW_Test_HasVulkan && SDW_HasCompilerSpirV && SDW_HasVulkanLayer
			if ( compilers.spirV
				&& testCounts.isInitialised( infoIndex ) )
			{
				testCounts.incIndent();
				testCounts << "Vulkan " << printVkVersion( testCounts.getVulkanVersion( infoIndex ) )
					<< " - SPIR-V " << printSpvVersion( testCounts.getSpirVVersion( infoIndex ) ) << endl;
				ast::vk::ProgramPipeline program{ shaders };
				std::string errors;
				auto isValidated = validateProgram( program, errors, testCounts, infoIndex );
				check( isValidated );
				check( errors.empty() );

				if ( !isValidated
					|| !errors.empty() )
				{
					if ( !errors.empty() )
					{
						testCounts << errors << endl;
					}

					auto validate = [&]( ast::Shader const & shader )
					{
						spirv::SpirVConfig config{};
						config.specVersion = testCounts.getSpirVVersion( infoIndex );
						auto sdwSpirV = spirv::serialiseSpirv( shader, config );
						auto crossGlsl = test::validateSpirVToGlsl( sdwSpirV
							, shader.getType()
							, testCounts );
						auto textSpirv = spirv::writeSpirv( shader, config );
						displayShader( "SPIR-V", textSpirv, testCounts, true );
						displayShader( "SpirV-Cross GLSL", crossGlsl, testCounts, true );
						auto glslangSpirv = compileGlslToSpv( shader.getType()
							, glsl::compileGlsl( shader
								, ast::SpecialisationInfo{}
								, getDefaultGlslConfig() ) );
						displayShader( "glslang SPIR-V"
							, spirv::displaySpirv( glslangSpirv )
							, testCounts
							, true );
					};

					for ( auto & shader : shaders )
					{
						checkNoThrow( validate( shader ) );
					}
				}

				testCounts.decIndent();
			}
#endif
		}

		void validateShaderOnIndex( ::ast::Shader const & shader
			, sdw_test::TestCounts & testCounts
			, uint32_t infoIndex
			, Compilers const & compilers )
		{
#if SDW_Test_HasVulkan && SDW_HasCompilerSpirV && SDW_HasVulkanLayer
			if ( compilers.spirV
				&& testCounts.isInitialised( infoIndex ) )
			{
				testCounts.incIndent();
				testCounts << "Vulkan " << printVkVersion( testCounts.getVulkanVersion( infoIndex ) )
					<< " - SPIR-V " << printSpvVersion( testCounts.getSpirVVersion( infoIndex ) ) << endl;
				ast::vk::ProgramPipeline program{ shader };
				std::string errors;
				auto isValidated = validateProgram( program, errors, testCounts, infoIndex );
				check( isValidated );
				check( errors.empty() );

				if ( !isValidated
					|| !errors.empty() )
				{
					if ( !errors.empty() )
					{
						testCounts << errors << endl;
					}

					auto validate = [&]()
					{
						spirv::SpirVConfig config{};
						config.specVersion = testCounts.getSpirVVersion( infoIndex );
						auto sdwSpirV = spirv::serialiseSpirv( shader, config );
						auto crossGlsl = test::validateSpirVToGlsl( sdwSpirV
							, shader.getType()
							, testCounts );
						auto textSpirv = spirv::writeSpirv( shader, config );
						displayShader( "SPIR-V"
							, textSpirv
							, testCounts
							, true );
						displayShader( "SpirV-Cross GLSL"
							, crossGlsl
							, testCounts
							, true );
						auto glslangSpirv = compileGlslToSpv( shader.getType()
							, glsl::compileGlsl( shader
								, ast::SpecialisationInfo{}
								, getDefaultGlslConfig() ) );
						displayShader( "glslang SPIR-V"
							, spirv::displaySpirv( glslangSpirv )
							, testCounts
							, true );
					};
					checkNoThrow( validate() );
				}

				testCounts.decIndent();
			}
#endif
		}
	}

	namespace sdw_test
	{
		TestSuite::TestSuite( std::string const & name )
			: test::TestSuite{ name }
		{
			initialiseGlslang();
		}
		
		TestSuite::~TestSuite()
		{
			cleanupGlslang();
		}

		TestCounts::TestCounts( test::TestSuite & psuite )
			: test::TestCounts{ psuite }
		{
		}

		void TestCounts::doInitialise()
		{
			createGLSLContext( *this );
			createHLSLContext( *this );
			createSPIRVContext( *this );
		}

		void TestCounts::doCleanup()
		{
			destroySPIRVContext( *this );
			destroyHLSLContext( *this );
			destroyGLSLContext( *this );
		}

		bool TestCounts::isInitialised( uint32_t infoIndex )const
		{
			return retrieveIsInitialised( *this, infoIndex );
		}

		uint32_t TestCounts::getVulkanVersion( uint32_t infoIndex )const
		{
			uint32_t ret{};

			if ( spirv )
			{
				ret = retrieveVulkanVersion( *this, infoIndex );
			}

			return ret;
		}

		uint32_t TestCounts::getSpirVVersion( uint32_t infoIndex )const
		{
			uint32_t ret{ 0x00010300 };

			if ( spirv )
			{
				ret = retrieveSPIRVVersion( *this, infoIndex );
			}

			return ret;
		}

		uint32_t TestCounts::getSpirvInfosSize()const
		{
			return spirv
				? retrieveSpirvInfosSize( *this )
				: 0u;
		}
	}

	void writeShader( ::ast::Shader const & shader
		, sdw_test::TestCounts & testCounts
		, Compilers const & compilers )
	{
		testCounts.incIndent();
		testCounts << "Write shader - " << printStage( shader.getType() ) << " stage" << endl;

		auto specialisation = getSpecialisationInfo( shader );
		testWriteDebug( shader, specialisation, compilers, testCounts );
		testWriteSpirV( shader, specialisation, compilers, testCounts );
		testWriteGlsl( shader, specialisation, compilers, testCounts );
		testWriteHlsl( shader, specialisation, compilers, testCounts );
		testCounts.decIndent();
	}

	void writeShader( sdw::ShaderWriter const & writer
		, sdw_test::TestCounts & testCounts
		, Compilers const & compilers )
	{
		writeShader( writer.getShader()
			, testCounts
			, compilers );
	}

	void expectError( std::string const & value
		, sdw_test::TestCounts & testCounts )
	{
		testCounts.expectedError = value;
	}

	void validateShaders( ast::ShaderArray const & shaders
		, sdw_test::TestCounts & testCounts
		, Compilers const & compilers )
	{
		testCounts.incIndent();
		testCounts << "Validate shaders" << endl;

		for ( uint32_t infoIndex = 0u; infoIndex < testCounts.getSpirvInfosSize(); ++infoIndex )
		{
			validateShadersOnIndex( shaders
				, testCounts
				, infoIndex
				, compilers );
		}

		testCounts.decIndent();
	}

	void validateShader( ast::Shader const & shader
		, sdw_test::TestCounts & testCounts
		, Compilers const & compilers )
	{
		testCounts.incIndent();
		testCounts << "Validate shader" << endl;

		for ( uint32_t infoIndex = 0u; infoIndex < testCounts.getSpirvInfosSize(); ++infoIndex )
		{
			validateShaderOnIndex( shader
				, testCounts
				, infoIndex
				, compilers );
		}

		testCounts.decIndent();
	}
}
