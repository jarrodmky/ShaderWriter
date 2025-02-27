#include "Common.hpp"
#include "WriterCommon.hpp"

#include <ShaderWriter/CompositeTypes/Struct.hpp>
#include <ShaderWriter/CompositeTypes/ArraySsbo.hpp>

namespace
{
#define DummyMain writer.implementMain( [&]( sdw::FragmentIn in, sdw::FragmentOut out ){} )

	template< typename T >
	void testStructuredSsbo( test::sdw_test::TestCounts & testCounts )
	{
		testBegin( "testStructuredSsbo" + ast::debug::getName( sdw::typeEnum< T > ) );
		{
			sdw::FragmentWriter writer;
			auto & shader = writer.getShader();
			std::string const name = "m_member" + sdw::debug::getName( sdw::typeEnum< T > );
			sdw::Struct type{ writer, "Type", ast::type::MemoryLayout::eStd140 };
			type.declMember< T >( name );
			type.end();
			sdw::ArraySsboT< sdw::StructInstance > bo{ writer, "Datas", type.getType(), 1u, 1u, true };
			auto value = bo[0].getMember< T >( name );
			check( getNonArrayKind( value.getType() ) == sdw::typeEnum< T > );
			check( getArraySize( value.getType() ) == sdw::type::NotArray );
			require( value.getExpr()->getKind() == sdw::expr::Kind::eMbrSelect );
			auto & stmt = *shader.getStatements()->back();
			require( stmt.getKind() == sdw::stmt::Kind::eShaderStructBufferDecl );
			check( static_cast< sdw::stmt::ShaderStructBufferDecl const & >( stmt ).getBindingPoint() == 1u );
			check( static_cast< sdw::stmt::ShaderStructBufferDecl const & >( stmt ).getDescriptorSet() == 1u );
			DummyMain;
			test::writeShader( writer, testCounts, CurrentCompilers );
		}
		{
			sdw::FragmentWriter writer;
			auto & shader = writer.getShader();
			std::string const name = "m_memberArray" + sdw::debug::getName( sdw::typeEnum< T > );
			sdw::Struct type{ writer, "Type", ast::type::MemoryLayout::eStd140 };
			type.declMember< T >( name, 4u );
			type.end();
			sdw::ArraySsboT< sdw::StructInstance > bo{ writer, "Datas", type.getType(), 1u, 1u, true };
			auto value = bo[0].getMemberArray< T >( name );
			check( getNonArrayKind( value.getType() ) == sdw::typeEnum< T > );
			check( getArraySize( value.getType() ) == 4u );
			require( value.getExpr()->getKind() == sdw::expr::Kind::eMbrSelect );
			auto & stmt = *shader.getStatements()->back();
			require( stmt.getKind() == sdw::stmt::Kind::eShaderStructBufferDecl );
			check( static_cast< sdw::stmt::ShaderStructBufferDecl const & >( stmt ).getBindingPoint() == 1u );
			check( static_cast< sdw::stmt::ShaderStructBufferDecl const & >( stmt ).getDescriptorSet() == 1u );
			DummyMain;
			test::writeShader( writer, testCounts, CurrentCompilers );
		}
		testEnd();
	}

	template< typename T >
	void testArraySsbo( test::sdw_test::TestCounts & testCounts )
	{
		testBegin( "testArraySsbo" + ast::debug::getName( sdw::typeEnum< T > ) );
		{
			sdw::FragmentWriter writer;
			auto & shader = writer.getShader();
			std::string const name = "m_member" + sdw::debug::getName( sdw::typeEnum< T > );
			sdw::ArraySsboT< T > bo{ writer, "Datas", T::makeType( shader.getTypesCache() ), ast::type::MemoryLayout::eStd140, 1u, 1u, true };
			auto value = bo[0];
			check( getNonArrayKind( value.getType() ) == sdw::typeEnum< T > );
			check( getArraySize( value.getType() ) == sdw::type::NotArray );
			require( value.getExpr()->getKind() == sdw::expr::Kind::eArrayAccess );
			auto & stmt = *shader.getStatements()->back();
			require( stmt.getKind() == sdw::stmt::Kind::eShaderStructBufferDecl );
			check( static_cast< sdw::stmt::ShaderStructBufferDecl const & >( stmt ).getBindingPoint() == 1u );
			check( static_cast< sdw::stmt::ShaderStructBufferDecl const & >( stmt ).getDescriptorSet() == 1u );
			DummyMain;
			test::writeShader( writer, testCounts, CurrentCompilers );
		}
		testEnd();
	}
}

sdwTestSuiteMain( TestWriterArraySsboDeclarations )
{
	sdwTestSuiteBegin();
	testStructuredSsbo< sdw::Int >( testCounts );
	testStructuredSsbo< sdw::UInt >( testCounts );
	testStructuredSsbo< sdw::Float >( testCounts );
	testStructuredSsbo< sdw::Double >( testCounts );
	testStructuredSsbo< sdw::Vec2 >( testCounts );
	testStructuredSsbo< sdw::Vec3 >( testCounts );
	testStructuredSsbo< sdw::Vec4 >( testCounts );
	testStructuredSsbo< sdw::DVec2 >( testCounts );
	testStructuredSsbo< sdw::DVec3 >( testCounts );
	testStructuredSsbo< sdw::DVec4 >( testCounts );
	testStructuredSsbo< sdw::IVec2 >( testCounts );
	testStructuredSsbo< sdw::IVec3 >( testCounts );
	testStructuredSsbo< sdw::IVec4 >( testCounts );
	testStructuredSsbo< sdw::UVec2 >( testCounts );
	testStructuredSsbo< sdw::UVec3 >( testCounts );
	testStructuredSsbo< sdw::UVec4 >( testCounts );
	testStructuredSsbo< sdw::Mat2 >( testCounts );
	testStructuredSsbo< sdw::Mat2x3 >( testCounts );
	testStructuredSsbo< sdw::Mat2x4 >( testCounts );
	testStructuredSsbo< sdw::Mat3 >( testCounts );
	testStructuredSsbo< sdw::Mat3x2 >( testCounts );
	testStructuredSsbo< sdw::Mat3x4 >( testCounts );
	testStructuredSsbo< sdw::Mat4 >( testCounts );
	testStructuredSsbo< sdw::Mat4x2 >( testCounts );
	testStructuredSsbo< sdw::Mat4x3 >( testCounts );
	testStructuredSsbo< sdw::DMat2 >( testCounts );
	testStructuredSsbo< sdw::DMat2x3 >( testCounts );
	testStructuredSsbo< sdw::DMat2x4 >( testCounts );
	testStructuredSsbo< sdw::DMat3 >( testCounts );
	testStructuredSsbo< sdw::DMat3x2 >( testCounts );
	testStructuredSsbo< sdw::DMat3x4 >( testCounts );
	testStructuredSsbo< sdw::DMat4 >( testCounts );
	testStructuredSsbo< sdw::DMat4x2 >( testCounts );
	testStructuredSsbo< sdw::DMat4x3 >( testCounts );
	testArraySsbo< sdw::Int >( testCounts );
	testArraySsbo< sdw::UInt >( testCounts );
	testArraySsbo< sdw::Float >( testCounts );
	testArraySsbo< sdw::Double >( testCounts );
	testArraySsbo< sdw::Vec2 >( testCounts );
	testArraySsbo< sdw::Vec3 >( testCounts );
	testArraySsbo< sdw::Vec4 >( testCounts );
	testArraySsbo< sdw::DVec2 >( testCounts );
	testArraySsbo< sdw::DVec3 >( testCounts );
	testArraySsbo< sdw::DVec4 >( testCounts );
	testArraySsbo< sdw::IVec2 >( testCounts );
	testArraySsbo< sdw::IVec3 >( testCounts );
	testArraySsbo< sdw::IVec4 >( testCounts );
	testArraySsbo< sdw::UVec2 >( testCounts );
	testArraySsbo< sdw::UVec3 >( testCounts );
	testArraySsbo< sdw::UVec4 >( testCounts );
	testArraySsbo< sdw::Mat2 >( testCounts );
	testArraySsbo< sdw::Mat2x3 >( testCounts );
	testArraySsbo< sdw::Mat2x4 >( testCounts );
	testArraySsbo< sdw::Mat3 >( testCounts );
	testArraySsbo< sdw::Mat3x2 >( testCounts );
	testArraySsbo< sdw::Mat3x4 >( testCounts );
	testArraySsbo< sdw::Mat4 >( testCounts );
	testArraySsbo< sdw::Mat4x2 >( testCounts );
	testArraySsbo< sdw::Mat4x3 >( testCounts );
	testArraySsbo< sdw::DMat2 >( testCounts );
	testArraySsbo< sdw::DMat2x3 >( testCounts );
	testArraySsbo< sdw::DMat2x4 >( testCounts );
	testArraySsbo< sdw::DMat3 >( testCounts );
	testArraySsbo< sdw::DMat3x2 >( testCounts );
	testArraySsbo< sdw::DMat3x4 >( testCounts );
	testArraySsbo< sdw::DMat4 >( testCounts );
	testArraySsbo< sdw::DMat4x2 >( testCounts );
	testArraySsbo< sdw::DMat4x3 >( testCounts );
	sdwTestSuiteEnd();
}

sdwTestSuiteLaunch( TestWriterArraySsboDeclarations )
