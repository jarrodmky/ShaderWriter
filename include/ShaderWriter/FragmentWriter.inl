/*
See LICENSE file in root folder
*/
#include "FragmentWriter.hpp"

namespace sdw
{
	//*************************************************************************

	template< template< ast::var::Flag FlagT > typename DataT >
	template< typename ... ParamsT >
	FragmentInT< DataT >::FragmentInT( ShaderWriter & writer
		, ast::FragmentOrigin origin
		, ast::FragmentCenter center
		, ParamsT ... params )
		: FragmentInT{ writer
			, makeExpr( writer
				, sdw::getShader( writer ).registerName( "fragIn"
					, makeFragmentInputType( makeType( getTypesCache( writer ), std::forward< ParamsT >( params )... )
						, origin
						, center )
					, FlagT ) ) }
	{
	}

	template< template< ast::var::Flag FlagT > typename DataT >
	template< typename ... ParamsT >
	FragmentInT< DataT >::FragmentInT( ShaderWriter & writer
		, ParamsT ... params )
		: FragmentInT{ writer
			, ast::FragmentOrigin::eUpperLeft
			, ast::FragmentCenter::eHalfPixel
			, std::forward< ParamsT >( params )... }
	{
	}

	template< template< ast::var::Flag FlagT > typename DataT >
	FragmentInT< DataT >::FragmentInT( ShaderWriter & writer
		, ast::expr::ExprPtr expr
		, bool enabled )
		: InputT< DataT >{ writer, std::move( expr ), enabled }
		, fragCoord{ getVec4Member( *this, ast::Builtin::eFragCoord ) }
		, frontFacing{ getBoolMember( *this, ast::Builtin::eFrontFacing ) }
		, pointCoord{ getVec2Member( *this, ast::Builtin::ePointCoord ) }
		, sampleID{ getIntMember( *this, ast::Builtin::eSampleID ) }
		, samplePosition{ getVec2Member( *this, ast::Builtin::eSamplePosition ) }
		, sampleMask{ getIntMemberArray( *this, ast::Builtin::eSampleMaskIn ) }
		, clipDistance{ getFloatMemberArray( *this, ast::Builtin::eClipDistance ) }
		, primitiveID{ getIntMember( *this, ast::Builtin::ePrimitiveID ) }
		, layer{ getIntMember( *this, ast::Builtin::eLayer ) }
		, viewportIndex{ getIntMember( *this, ast::Builtin::eViewportIndex ) }
	{
	}

	template< template< ast::var::Flag FlagT > typename DataT >
	template< typename ... ParamsT >
	ast::type::IOStructPtr FragmentInT< DataT >::makeType( ast::type::TypesCache & cache
		, ParamsT ... params )
	{
		auto result = InputT< DataT >::makeType( cache, std::forward< ParamsT >( params )... );

		if ( !result->hasMember( ast::Builtin::eFragCoord ) )
		{
			result->declMember( ast::Builtin::eFragCoord
				, type::Kind::eVec4F
				, ast::type::NotArray );
			result->declMember( ast::Builtin::eFrontFacing
				, type::Kind::eBoolean
				, ast::type::NotArray );
			result->declMember( ast::Builtin::ePointCoord
				, type::Kind::eVec2F
				, ast::type::NotArray );
			result->declMember( ast::Builtin::eSampleID
				, type::Kind::eInt
				, ast::type::NotArray );
			result->declMember( ast::Builtin::eSamplePosition
				, type::Kind::eVec2F
				, ast::type::NotArray );
			result->declMember( ast::Builtin::eSampleMaskIn
				, type::Kind::eInt
				, 8u );
			result->declMember( ast::Builtin::eClipDistance
				, type::Kind::eFloat
				, 8u );
			result->declMember( ast::Builtin::ePrimitiveID
				, type::Kind::eInt
				, ast::type::NotArray );
			result->declMember( ast::Builtin::eLayer
				, type::Kind::eInt
				, ast::type::NotArray );
			result->declMember( ast::Builtin::eViewportIndex
				, type::Kind::eInt
				, ast::type::NotArray );
		}

		return result;
	}

	//*************************************************************************

	template< template< ast::var::Flag FlagT > typename DataT >
	template< typename ... ParamsT >
	FragmentOutT< DataT >::FragmentOutT( ShaderWriter & writer
		, ParamsT ... params )
		: FragmentOutT{ writer
			, makeExpr( writer
				, sdw::getShader( writer ).registerName( "fragOut"
					, makeType( getTypesCache( writer ), std::forward< ParamsT >( params )... )
					, FlagT ) ) }
	{
	}

	template< template< ast::var::Flag FlagT > typename DataT >
	FragmentOutT< DataT >::FragmentOutT( ShaderWriter & writer
		, ast::expr::ExprPtr expr
		, bool enabled )
		: OutputT< DataT >{ writer, std::move( expr ), enabled }
		, fragDepth{ getFloatMember( *this, ast::Builtin::eFragDepth ) }
		, sampleMask{ getIntMemberArray( *this, ast::Builtin::eSampleMask ) }
	{
	}

	template< template< ast::var::Flag FlagT > typename DataT >
	template< typename ... ParamsT >
	ast::type::IOStructPtr FragmentOutT< DataT >::makeType( ast::type::TypesCache & cache
		, ParamsT ... params )
	{
		auto result = OutputT< DataT >::makeType( cache, std::forward< ParamsT >( params )... );

		if ( !result->hasMember( ast::Builtin::eFragDepth ) )
		{
			result->declMember( ast::Builtin::eFragDepth
				, type::Kind::eFloat
				, ast::type::NotArray );
			result->declMember( ast::Builtin::eSampleMask
				, type::Kind::eInt
				, 8u );
		}

		return result;
	}

	//*************************************************************************

	/**
	*name
	*	I/O layout declaration.
	*/
	/**@{*/
	template< template< ast::var::Flag FlagT > typename InT
		, template< ast::var::Flag FlagT > typename OutT >
	inline void FragmentWriter::implementMainT( FragmentMainFuncT< InT, OutT > const & function )
	{
		this->implementMainT( ast::FragmentOrigin::eUpperLeft
			, ast::FragmentCenter::eHalfPixel
			, function );
	}

	template< template< ast::var::Flag FlagT > typename InT
		, template< ast::var::Flag FlagT > typename OutT >
	inline void FragmentWriter::implementMainT( ast::FragmentOrigin origin
		, ast::FragmentCenter center
		, FragmentMainFuncT< InT, OutT > const & function )
	{
		this->implementMainT( FragmentInT< InT >{ *this, origin, center }
			, FragmentOutT< OutT >{ *this }
			, function );
	}

	template< template< ast::var::Flag FlagT > typename InT
		, template< ast::var::Flag FlagT > typename OutT >
	inline void FragmentWriter::implementMainT( FragmentInT< InT > in
		, FragmentOutT< OutT > out
		, FragmentMainFuncT< InT, OutT > const & function )
	{
		( void )implementFunction< Void >( "main"
			, ast::stmt::FunctionFlag::eEntryPoint
			, function
			, makeInParam( std::move( in ) )
			, makeOutParam( std::move( out ) ) );
	}
	/**@}*/
	/**
	*name
	*	Blend Output declaration.
	*/
	/**@{*/
	template< typename T >
	inline T FragmentWriter::declBlendOutput( std::string const & name
		, uint32_t location
		, uint32_t blendIndex
		, bool enabled )
	{
		return declBlendOutput< T >( name
			, location
			, blendIndex
			, 0u
			, enabled );
	}

	template< typename T >
	inline T FragmentWriter::declBlendOutput( std::string const & name
		, uint32_t location
		, uint32_t blendIndex
		, uint32_t attributes
		, bool enabled )
	{
		static_assert( !IsSameV< T, Boolean >, "Bool is not supported as output type" );
		static_assert( !IsSameV< T, BVec2 >, "BVec2 is not supported as output type" );
		static_assert( !IsSameV< T, BVec3 >, "BVec3 is not supported as output type" );
		static_assert( !IsSameV< T, BVec4 >, "BVec4 is not supported as output type" );
		static_assert( !IsSameV< T, Double >, "Double is not supported as output type" );
		static_assert( !IsSameV< T, DVec2 >, "DVec2 is not supported as output type" );
		static_assert( !IsSameV< T, DVec3 >, "DVec3 is not supported as output type" );
		static_assert( !IsSameV< T, DVec4 >, "DVec4 is not supported as output type" );
		auto type = T::makeType( getTypesCache() );
		auto var = registerOutput( name
			, location
			, attributes | var::Flag::eBlendIndex
			, type );

		if ( enabled )
		{
			addStmt( sdw::makeInOutBlendVariableDecl( var
				, location
				, blendIndex ) );
		}

		return T{ *this
			, makeExpr( *this, var )
			, enabled };
	}

	template< typename T >
	inline Array< T > FragmentWriter::declBlendOutputArray( std::string const & name
		, uint32_t location
		, uint32_t blendIndex
		, uint32_t dimension
		, bool enabled )
	{
		return declBlendOutputArray< T >( name
			, location
			, blendIndex
			, dimension
			, 0u
			, enabled );
	}

	template< typename T >
	inline Array< T > FragmentWriter::declBlendOutputArray( std::string const & name
		, uint32_t location
		, uint32_t blendIndex
		, uint32_t dimension
		, uint32_t attributes
		, bool enabled )
	{
		static_assert( !IsSameV< T, Boolean >, "Bool is not supported as output type" );
		static_assert( !IsSameV< T, BVec2 >, "BVec2 is not supported as output type" );
		static_assert( !IsSameV< T, BVec3 >, "BVec3 is not supported as output type" );
		static_assert( !IsSameV< T, BVec4 >, "BVec4 is not supported as output type" );
		static_assert( !IsSameV< T, Double >, "Double is not supported as output type" );
		static_assert( !IsSameV< T, DVec2 >, "DVec2 is not supported as output type" );
		static_assert( !IsSameV< T, DVec3 >, "DVec3 is not supported as output type" );
		static_assert( !IsSameV< T, DVec4 >, "DVec4 is not supported as output type" );
		auto type = Array< T >::makeType( getTypesCache()
			, dimension );
		auto var = registerOutput( name
			, location
			, blendIndex
			, attributes | var::Flag::eBlendIndex
			, type );

		if ( enabled )
		{
			addStmt( sdw::makeInOutBlendVariableDecl( var
				, location
				, blendIndex ) );
		}

		return Array< T >{ *this
			, makeExpr( *this, var )
			, enabled };
	}
	/**@}*/

	//*************************************************************************
}
