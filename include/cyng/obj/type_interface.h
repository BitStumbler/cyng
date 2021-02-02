/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_OBJ_TYPE_INTERFACE_H
#define DOCC_OBJ_TYPE_INTERFACE_H

#include <cstdint>
#include <cstddef>
#include <typeinfo>

namespace docscript {

	class type_interface
	{
	public:
		/**	@brief  simple C++ runtime type identification (RTTI).
		 *
		 * Make sure RTTI is enabled
		 *
		 *  @return runtime type identification (RTTI) provided by C++ compiler
		 */
		virtual const std::type_info& rtti() const = 0;

		/**
		 * This is the result of the sizeof() operator.
		 *
		 * @return Storage size in bytes
		 */
		virtual std::size_t size() const = 0;

		/**
		 * @return Dimension of the array
		 */
		virtual std::size_t extend() const = 0;

		/**
		 * Checks whether T is an integral type.
		 */
		virtual bool is_integral() const = 0;

		/**
		 * Checks whether T is a floating-point type.
		 */
		virtual bool is_floating_point() const = 0;

		/**
		 * Checks whether T is an enumeration type.
		 */
		virtual bool is_enum() const = 0;

		/**
		 * Checks whether T is const-qualified
		 */
		virtual bool is_const() const = 0;


		/**
		 * This is the position in the global type tuple.
		 * 
		 * @return The internal type tag (type_code)
		 */
		virtual std::size_t tag() const = 0;

		/**
		 * @return the type if for built-in types
		 */
		virtual const char* type_name() const = 0;

	};
}
#endif