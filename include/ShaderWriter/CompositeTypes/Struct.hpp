/*
See LICENSE file in root folder
*/
#ifndef ___SDW_Struct_H___
#define ___SDW_Struct_H___

#include "StructInstance.hpp"

#include <ShaderAST/Stmt/StmtStructureDecl.hpp>

namespace sdw
{
	class Struct
	{
	public:
		SDW_API Struct( ShaderWriter & writer
			, std::string name
			, type::MemoryLayout layout = type::MemoryLayout::eStd140 );
		SDW_API Struct( ShaderWriter & writer
			, ast::type::BaseStructPtr type );
		SDW_API void end();
		template< typename InstanceT = StructInstance >
		InstanceT getInstance( std::string const & name
			, bool enabled )const;

		type::BaseStructPtr getType()const
		{
			return m_type;
		}

		template< typename T >
		inline void declMember( std::string const & name );
		template< typename T >
		inline void declMember( std::string const & name
			, uint32_t dimension );
		SDW_API void declMember( std::string name
			, Struct const & type );

	private:
		ShaderWriter * m_writer;
		Shader * m_shader;
		type::BaseStructPtr m_type;
	};

	template< var::Flag FlagT >
	class IOStructT
	{
	private:
		IOStructT( ShaderWriter & writer
			, std::string name );
		IOStructT( ShaderWriter & writer
			, ast::type::IOStructPtr type );
		void end();

		type::IOStructPtr getType()const
		{
			return m_type;
		}

		template< typename T >
		inline void declMember( std::string const & name
			, uint32_t location );
		template< typename T >
		inline void declMember( std::string const & name
			, uint32_t dimension
			, uint32_t location );

	private:
		ShaderWriter * m_writer;
		Shader * m_shader;
		type::IOStructPtr m_type;
	};

	using InputStruct = IOStructT< var::Flag::eShaderInput >;
	using OutputStruct = IOStructT< var::Flag::eShaderOutput >;
}

#include "Struct.inl"

#endif
