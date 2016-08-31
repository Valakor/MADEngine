#pragma once

#include <cassert>

#define MAD_ASSERT_DESC(expression, desc)	\
	assert((expression) && (desc));
