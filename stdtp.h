//-< STDTP.H >-------------------------------------------------------*--------*
// GOODS                     Version 1.0         (c) 1997  GARRET    *     ?  *
// (Generic Object Oriented Database System)                         *   /\|  *
//                                                                   *  /  \  *
//                          Created:      7-Jan-97    K.A. Knizhnik  * / [] \ *
//                          Last update:  2-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Standart type and macro definitions
//-------------------------------------------------------------------*--------*

#ifndef __STDTP_H__
#define __STDTP_H__

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>

#if defined(_WIN32)
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(disable:4275)
#endif
#endif

#ifdef POST_DLL
#ifdef INSIDE_POST
#define POST_DLL_ENTRY __declspec(dllexport)
#else
#define POST_DLL_ENTRY __declspec(dllimport)
#endif
#else
#define POST_DLL_ENTRY
#endif

#ifdef USE_NAMESPACES
#define POST_NAMESPACE post::
#define BEGIN_POST_NAMESPACE namespace post {
#define END_POST_NAMESPACE }
#define USE_POST_NAMESPACE using namespace post;
#else
#define POST_NAMESPACE
#define BEGIN_POST_NAMESPACE
#define END_POST_NAMESPACE
#define USE_POST_NAMESPACE 
#endif

BEGIN_POST_NAMESPACE

//
// This are different level of debugging
//
#define DEBUG_NONE  0
#define DEBUG_CHECK 1
#define DEBUG_TRACE 2

#ifndef DEBUG_LEVEL
#ifdef NDEBUG
#define DEBUG_LEVEL DEBUG_NONE
#else
#define DEBUG_LEVEL DEBUG_CHECK
#endif
#endif

// Align value 'x' to boundary 'b' which should be power of 2
#define ALIGN(x,b)   (((x) + (b) - 1) & ~((b) - 1))

typedef signed char    int1;
typedef unsigned char  nat1;

typedef signed short   int2;
typedef unsigned short nat2;

typedef signed int     int4;
typedef unsigned int   nat4;

#if defined(_WIN32)
typedef unsigned __int64 nat8;
typedef __int64          int8;
#define INT8_FORMAT "I64"
#define CONST64(c)  c
#else
#if defined(__osf__ )
typedef unsigned long nat8;
typedef signed   long int8;
#define INT8_FORMAT "l"
#define CONST64(c)  c##L
#else
#if defined(__GNUC__) || defined(__SUNPRO_CC)
typedef unsigned long long nat8;
typedef signed   long long int8;
#define INT8_FORMAT "ll"
#define CONST64(c)  c##LL
#else
#error "integer 8 byte type is not defined" 
#endif
#endif
#endif

#define nat8_low_part(x)  ((nat4)(x))
#define nat8_high_part(x) ((nat4)((nat8)(x)>>32))
#define int8_low_part(x)  ((int4)(x))
#define int8_high_part(x) ((int4)((int8)(x)>>32))
#define cons_nat8(hi, lo) ((((nat8)(hi)) << 32) | (nat4)(lo))
#define cons_int8(hi, lo) ((((int8)(hi)) << 32) | (nat4)(lo))
 
#define MAX_NAT8  nat8(-1)

typedef float  real4;
typedef double real8; 

#define items(array) (sizeof(array)/sizeof*(array))

#if DEBUG_LEVEL >= DEBUG_TRACE
#define TRACE_MSG(msg) printf msg
#else
#define TRACE_MSG(msg)
#endif

END_POST_NAMESPACE

#endif




