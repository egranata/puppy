#ifndef alltypes_h
#define alltypes_h

#define _Addr int
#define _Int64 long long
#define _Reg int

#if __GNUC__ >= 3
typedef __builtin_va_list va_list;
typedef __builtin_va_list __isoc_va_list;
#else
typedef struct __va_list * va_list;
typedef struct __va_list * __isoc_va_list;
#endif

#ifndef __cplusplus
#ifdef __WCHAR_TYPE__
typedef __WCHAR_TYPE__ wchar_t;
#else
typedef long wchar_t;
#endif
#endif

#if defined(__FLT_EVAL_METHOD__) && __FLT_EVAL_METHOD__ == 0
typedef float float_t;
typedef double double_t;
#else
typedef long double float_t;
typedef long double double_t;
#endif

typedef struct { __attribute__((__aligned__(8))) long long __ll; long double __ld; } max_align_t;

typedef long time_t;
typedef long suseconds_t;
typedef long unsigned int size_t;

#endif
