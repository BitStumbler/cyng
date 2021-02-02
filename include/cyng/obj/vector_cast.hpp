/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_OBJ_VECTOR_CAST_H
#define DOCC_OBJ_VECTOR_CAST_H

#include <cyng/obj/value_cast.hpp>
#include <vector> 
#include <algorithm>
#include <iterator>

#ifdef _DEBUG
#include <cyng/io/ostream.h>
#include <cyng/io/ostream.h>
#endif

#include <boost/assert.hpp>

namespace docscript {

	template < typename T >
	[[nodiscard]]
	std::vector< T > vector_cast(vector_t const& vec, T const& def) noexcept
	{
		std::vector< T > result;
		result.reserve(vec.size());
		std::transform(std::begin(vec), std::end(vec), std::back_inserter(result), [&def](object const& obj) {
			return value_cast<T>(obj, def);
			});
		return result;
	}

	/**
	 * Read a vector of the specified data type.
	 *
	 * example
	 * @code
	  std::vector<std::string> vec = vector_cast<std::string>(obj, "");
	 * @endcode
	 */
	template < typename T >
	[[nodiscard]]
	std::vector< T > vector_cast(object const& obj, T const& def) noexcept
	{
		auto const* vp = object_cast<vector_t>(obj);
		return (vp != nullptr)
			? vector_cast<T>(*vp, def)
			: std::vector< T >{}
			;
	}
}

#endif //	DOCC_OBJ_VECTOR_CAST_H