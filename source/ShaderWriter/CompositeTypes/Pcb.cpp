/*
See LICENSE file in root folder
*/
#include "ShaderWriter/CompositeTypes/Pcb.hpp"

#include "ShaderWriter/Writer.hpp"
#include "ShaderWriter/CompositeTypes/Struct.hpp"
#include "ShaderWriter/CompositeTypes/StructInstance.hpp"

namespace sdw
{
	Pcb::Pcb( ShaderWriter & writer
		, std::string const & name
		, ast::type::MemoryLayout layout
		, bool enabled )
		: m_writer{ writer }
		, m_shader{ m_writer.getShader() }
		, m_stmt{ stmt::makePushConstantsBufferDecl( name, layout ) }
		, m_name{ name }
		, m_info{ writer.getTypesCache(), layout, name }
		, m_var{ writer.getShader().registerName( m_name, m_info.getType(), var::Flag::ePushConstant ) }
		, m_enabled{ enabled }
	{
	}

	void Pcb::end()
	{
		if ( isEnabled() )
		{
			addStmt( m_shader, std::move( m_stmt ) );
			m_shader.registerPcb( m_name, m_info );
		}
	}

	StructInstance Pcb::declMember( std::string const & name
		, Struct const & s
		, bool enabled )
	{
		auto type = m_info.registerMember( name, s.getType() );
		auto var = registerMember( m_writer, m_var, name, type );

		if ( isEnabled() && enabled )
		{
			m_stmt->add( stmt::makeVariableDecl( var ) );
		}

		return StructInstance{ m_writer
			, makeExpr( m_writer, var )
			, isEnabled() && enabled };
	}
}
