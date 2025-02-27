/*
See LICENSE file in root folder
*/
namespace sdw
{
	template< typename T >
	Sampler & Sampler::operator=( T const & rhs )
	{
		this->updateContainer( rhs );
		auto & shader = findWriterMandat( *this, rhs );

		if ( areOptionalEnabled( *this, rhs ) )
		{
			addStmt( shader
				, sdw::makeSimple( sdw::makeAssign( getExpr()->getType()
					, makeExpr( shader, getExpr() )
					, makeExpr( shader, rhs ) ) ) );
		}

		return *this;
	}
}
