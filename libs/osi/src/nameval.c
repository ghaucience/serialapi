#include <string.h>
#include "nameval.h"

int lookup_by_name(const struct name_val *table, const char *name)
{
	for (; table->name != NULL; table++) {
		if (!strcasecmp(table->name, name)) {
			return table->val;
		}
	}
	return -1;
}
const char *lookup_by_val(const struct name_val *table, int val)
{
	for (; table->name != NULL; table++) {
		if (table->val == val) {
			return table->name;
		}
	}
	return NULL;
}
