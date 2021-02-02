/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SYS_MEMORY_H
#define DOCC_SYS_MEMORY_H

#include <cstdint>

namespace docscript {
	namespace sys
	{
		/**
		 * @return available memory in system in bytes
		 */
		std::uint64_t get_total_physical_memory();
	}
}
#endif
