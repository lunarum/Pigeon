#ifndef CSTRING_H
#define CSTRING_H

#define CSTRING_NULL 0


typedef unsigned int cstring;

cstring cstring_add(const char * str);
const char *cstring_get(cstring s);

#endif // !CSTRING_H

