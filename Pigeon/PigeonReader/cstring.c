#include "cstring.h"
#include <malloc.h>
#include <string.h>

// define maximum buckets; must fit in unsigned int and must be a (binary) all 1's number
#define MAX_STRINGS 65535

char *cstrings[MAX_STRINGS];


/// <summary>
/// hash - generate a hash from a string, based on the FNV1 hashing algorithm (see http://www.isthe.com/chongo/tech/comp/fnv/index.html)
/// </summary>
/// <param name="str">the (constant) character string to be added</param>
/// <returns>
/// the hash of the copied / saved string <paramref name="str"/>
/// </returns>

/*
 * hash - generate a hash from a string, based on the FNV1 (32-bit version) hashing algorithm (see http://www.isthe.com/chongo/tech/comp/fnv/index.html)
 */
cstring hash(const char * str) {
	cstring h = 2166136261;
	for (const char *s = str; *s; ++s) {
		h ^= *s;
		h *= 16777619;
	}
	h &= MAX_STRINGS;	// trim down hash to (current) 16 bit
	if (h == CSTRING_NULL)
		++h;

	return h;
}

/// <summary>
/// cstring_add - add (a copy off) the given string <paramref name="str"/> to the internal hastable where duplicates are automatically catched
/// </summary>
/// <param name="str">the (constant) character string to be added</param>
/// <returns>
/// the hash of the copied / saved string <paramref name="str"/>
/// </returns>
cstring cstring_add(const char * str) {
	cstring s = hash(str);
	cstring t = s;
	size_t l = strlen(str) + 1;

	do {
		if (cstrings[s] == NULL) {	// empty bucket?
			if ((cstrings[s] = malloc(l)) != NULL)
				strcpy_s(cstrings[s], l, str);
			else
				s = CSTRING_NULL;
			break;
		}
		if (strcmp(cstrings[s], str) == 0)	// same string already stored?
			break;
		s = (s + 1) & MAX_STRINGS;	// check next bucket
		if (s == CSTRING_NULL)		// skip "NULL" cstring
			++s;
	} while (s != t);	// quit when all buckets are full

	return s;
}

/// <summary>
/// cstring_get - return the (constant) string which correspondes to the given hash <paramref name="s"/> to the internal hastable where duplicates are automatically catched
/// </summary>
/// <param name="s">the (hash of the) string to be returned</param>
/// <returns>
/// a pointer to de (constant) string
/// </returns>
const char *cstring_get(cstring s) {
	return s == CSTRING_NULL ? "?" : cstrings[s & MAX_STRINGS];
}