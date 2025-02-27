/*
See LICENSE file in root folder
*/
#ifndef ___SDW_StructuredSsbo_H___
#define ___SDW_StructuredSsbo_H___

#include "ShaderWriter/BaseTypes/UInt.hpp"

#include <ShaderAST/BoInfo.hpp>
#include <ShaderAST/Stmt/StmtShaderBufferDecl.hpp>

namespace sdw
{
	template< typename InstanceT >
	class ArraySsboT
	{
	public:
		ArraySsboT( ShaderWriter & writer
			, std::string const & name
			, ast::type::TypePtr dataType
			, ast::type::MemoryLayout layout
			, uint32_t bind
			, uint32_t set
			, bool enabled );

		ArraySsboT( ShaderWriter & writer
			, std::string const & name
			, ast::type::BaseStructPtr dataType
			, uint32_t bind
			, uint32_t set
			, bool enabled );

		ArraySsboT( ShaderWriter & writer
			, std::string const & name
			, uint32_t bind
			, uint32_t set
			, bool enabled );

		InstanceT operator[]( uint32_t index );
		InstanceT operator[]( UInt const & index );

		bool isEnabled()const
		{
			return m_enabled;
		}

	private:
		ShaderWriter & m_writer;
		ast::Shader & m_shader;
		std::string m_name;
		ast::InterfaceBlock m_interface;
		ast::SsboInfo m_info;
		type::BaseStructPtr m_ssboType;
		var::VariablePtr m_dataVar;
		var::VariablePtr m_ssboVar;
		bool m_enabled;
	};

	SDW_API void registerSsbo( ShaderWriter & writer
		, std::string const & name
		, SsboInfo const & info );
}

#include "ArraySsbo.inl"

#endif
