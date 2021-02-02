/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_STORE_RECORD_H
#define DOCC_STORE_RECORD_H

#include <cyng/store/key.hpp>
#include <cyng/store/meta.h>

#include <functional>
#include <boost/uuid/uuid.hpp>

namespace docscript {

	class record 
	{
	public:
		record() = delete;
		record(record const&) = default;
		record(meta_store const&);
		record(meta_store const&, key_t const& key, data_t const& data, std::uint64_t gen);

		/**
		 * @return true if record doesn't contain valid data
		 *
		 * @code
		 meta_.size() != (key().size() + body().size())
		 * @endcode
		 */
		bool empty() const;

		/**
		 * @return the key of the record
		 */
		key_t const& key() const;

		/**
		 * @return the data part of the record.
		 */
		data_t const& data() const;

		/*
		 * @return generation counter
		 */
		std::uint64_t get_generation() const;

		/**
		 * Convert the record into a tuple. Usefull for
		 * serialization to JSON, XML, you name it.
		 * If no columns names available, the column index is used
		 * as parameter name.
		 */
		tuple_t to_tuple() const;

		/**
		 * access by column name
		 */
		object at(std::string) const;

		/**
		 * access by column index (pk included)
		 */
		object at(std::size_t) const;

	private:
		meta_store const meta_;
		key_t const key_;
		data_t const data_;
		std::uint64_t const gen_;
	};
}
#endif
