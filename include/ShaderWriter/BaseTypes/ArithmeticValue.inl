/*
See LICENSE file in root folder
*/
namespace sdw
{
	//*************************************************************************

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT >::ArithmeticValue( ShaderWriter & writer
		, expr::ExprPtr expr
		, bool enabled )
		: Value{ writer, std::move( expr ), enabled }
	{
		assert( this->getType()->getKind() == KindT );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT >::ArithmeticValue( CppTypeT< ArithmeticValue< KindT > > rhs )
		: Value{ sdw::getCurrentWriter(), makeExpr( sdw::getCurrentWriter(), rhs ), true }
	{
		assert( this->getType()->getKind() == KindT );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT >::ArithmeticValue( Value rhs )
		: Value{ ctorCast< ArithmeticValue< KindT >, 1u >( std::move( rhs ) ) }
	{
		assert( this->getType()->getKind() == KindT );
	}

	template< ast::type::Kind KindT >
	template< typename T >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator=( T const & rhs )
	{
		static_assert( KindT == typeEnum< T > );
		this->updateContainer( rhs );
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator=( CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	expr::ExprPtr ArithmeticValue< KindT >::makeCondition()const
	{
		auto & writer = findWriterMandat( *this );
		return sdw::makeNEqual( makeExpr( writer, *this )
			, makeExpr( writer, CppTypeT< ArithmeticValue< KindT > >{} ) );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator+=( ArithmeticValue< KindT > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeAddAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator-=( ArithmeticValue< KindT > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeMinusAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator*=( ArithmeticValue< KindT > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeTimesAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator/=( ArithmeticValue< KindT > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeDivideAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator+=( CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeAddAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator-=( CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeMinusAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator*=( CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeTimesAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > & ArithmeticValue< KindT >::operator/=( CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		writeAssignOperator< ArithmeticValue< KindT > >( *this, rhs, sdw::makeDivideAssign );
		return *this;
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > ArithmeticValue< KindT >::operator-()const
	{
		return writeUnOperator< ArithmeticValue< KindT > >( *this, sdw::makeUnMinus );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > ArithmeticValue< KindT >::operator+()const
	{
		return writeUnOperator< ArithmeticValue< KindT > >( *this, sdw::makeUnPlus );
	}

	template< ast::type::Kind KindT >
	ast::type::TypePtr ArithmeticValue< KindT >::makeType( ast::type::TypesCache & cache )
	{
		return cache.getBasicType( KindT );
	}

	//*************************************************************************

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator+( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeAdd );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator-( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeMinus );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator*( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeTimes );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator/( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeDivide );
	}

	template< ast::type::Kind KindT >
	Boolean operator==( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator!=( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeNEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator<( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeLess );
	}

	template< ast::type::Kind KindT >
	Boolean operator<=( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeLEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator>( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeGreater );
	}

	template< ast::type::Kind KindT >
	Boolean operator>=( ArithmeticValue< KindT > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeGEqual );
	}

	//*************************************************************************

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator+( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeAdd );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator-( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeMinus );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator*( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeTimes );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator/( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeDivide );
	}

	template< ast::type::Kind KindT >
	Boolean operator==( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator!=( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeNEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator<( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeLess );
	}

	template< ast::type::Kind KindT >
	Boolean operator<=( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeLEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator>( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeGreater );
	}

	template< ast::type::Kind KindT >
	Boolean operator>=( ArithmeticValue< KindT > const & lhs
		, CppTypeT< ArithmeticValue< KindT > > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeGEqual );
	}

	//*************************************************************************

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator+( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeAdd );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator-( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeMinus );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator*( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeTimes );
	}

	template< ast::type::Kind KindT >
	ArithmeticValue< KindT > operator/( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeBinOperator< ArithmeticValue< KindT > >( lhs, rhs, sdw::makeDivide );
	}

	template< ast::type::Kind KindT >
	Boolean operator==( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator!=( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeNEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator<( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeLess );
	}

	template< ast::type::Kind KindT >
	Boolean operator<=( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeLEqual );
	}

	template< ast::type::Kind KindT >
	Boolean operator>( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeGreater );
	}

	template< ast::type::Kind KindT >
	Boolean operator>=( CppTypeT< ArithmeticValue< KindT > > const & lhs
		, ArithmeticValue< KindT > const & rhs )
	{
		return writeComparator< Boolean >( lhs, rhs, sdw::makeGEqual );
	}

	//*************************************************************************
}
