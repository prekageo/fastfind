#include "config.h"
#include "types.h"
#include "endians.h"
#include "support.h"

//ntfschar NTFS_INDEX_I30[5] = { const_cpu_to_le16('$'), const_cpu_to_le16('I'),
//                const_cpu_to_le16('3'), const_cpu_to_le16('0'),
//                const_cpu_to_le16('\0') };
//ntfschar AT_UNNAMED[] = { const_cpu_to_le16('\0') };

unsigned int test_and_set_bit_compat(unsigned int bit, unsigned int var) {
	const BOOL old_state = test_bit(bit, var);
	set_bit(bit, var);
	return old_state;
}

unsigned int test_and_clear_bit_compat(unsigned int bit, unsigned int var) {
	const BOOL old_state = test_bit(bit, var);
	clear_bit(bit, var);
	return old_state;
}
