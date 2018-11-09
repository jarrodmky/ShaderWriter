/*
See LICENSE file in root folder
*/
#ifndef ___AST_GlslTextureAccessFunctions_H___
#define ___AST_GlslTextureAccessFunctions_H___
#pragma once

#include "GlslHelpers.hpp"

namespace sdw::glsl
{
	void writeGlslTextureAccessFunctions( ast::stmt::Container * container
		, IntrinsicsConfig const & config );
}

#endif
