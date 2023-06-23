#ifndef BANNED_C_FUNCTIONS_H
#define BANNED_C_FUNCTIONS_H

/*
* This header lists functions that have been banned from our code base,
* because they're too easy to misuse (and even if used correctly,
* complicate audits). Including this header turns them into compile-time
* errors.
 *Source:
 * https://github.com/git/git/blob/master/banned.h
 *
 * Also used:
 * https://wiki.openstack.org/wiki/StarlingX/Security/Banned_C_Functions
*/

#define BANNED(func) sorry_##func##_is_a_banned_function

#undef strcpy
#define strcpy(x,y) BANNED(strcpy)
#undef wcscpy
#define wcscpy(x,y) BANNED(wcscpy)
#undef strcat
#define strcat(x,y) BANNED(strcat)
#undef wcscat
#define wcscat (x, y) BANNED(wcscat)
//#undef strncpy
//#define strncpy(x,y,n) BANNED(strncpy)
//#undef strncat
//#define strncat(x,y,n) BANNED(strncat)

#undef sprintf
#define sprintf(...) BANNED(sprintf)
#undef vsprintf
#define vsprintf(...) BANNED(vsprintf)
#undef vsnprintf
#define vsnprintf(...) BANNED(vsnprintf)

#undef strtok
#define strtok(x, Y) BANNED(strtok))

//banned except with approval of core. Requires detailed inspection to ensure
//field widths are specified. For unknown inputs, it is recommended to use
//strto* functions to avoid arithmetic overflows.
#undef scanf
#define scanf(...) BANNED(scanf)
#undef sscanf
#define sscanf(...) BANNED(sscanf)
#undef vscanf
#define vscanf(...) BANNED(vscanf)
#undef vsscanf
#define vsscanf(...) BANNED(vsscanf)

//unbounded, banned, use fgets() instead
#undef gets
#define gets(...) BANNED(gets)

//Use equivalent strto* functions
#undef atol
#define atol(...) BANNED(atol)
#undef atoll
#define atoll(...) BANNED(atoll)
#undef atof
#define atof(...) BANNED(atof)

#undef gmtime
#define gmtime(t) BANNED(gmtime)
#undef localtime
#define localtime(t) BANNED(localtime)
#undef ctime
#define ctime(t) BANNED(ctime)
#undef ctime_r
#define ctime_r(t, buf) BANNED(ctime_r)
#undef asctime
#define asctime(t) BANNED(asctime)
#undef asctime_r
#define asctime_r(t, buf) BANNED(asctime_r)

// TODO add malloc, calloc and realloc after first assignment
#undef alloca
#define alloca(...) BANNED(alloca)

#endif //BANNED_C_FUNCTIONS_H
