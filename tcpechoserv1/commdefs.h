/**************************************************************************

	Header file to present some common defines.

	Created by Chj on 2002.11.28

**************************************************************************/
// chjtest-for-cvsmailer1

#ifndef __CHJ_COMMDEFS_H
#define __CHJ_COMMDEFS_H

// #include <stddef.h>


typedef signed char Schar;

typedef unsigned int Uint;
typedef unsigned char Uchar;
typedef unsigned char Uint8;
	/* How come two names for "unsigned char"? `Uchar' implies readable 
	characters, i.e. probably not including those control chars;
	while Uint8 implies a binary data ranging from 0~255.	*/
typedef unsigned long Ulong;
typedef unsigned short Ushort;
typedef int RE_CODE;	
	// RE_CODE: result code, normally 0 as "no error" if using this type,
	//some value other than 0 means an error code.

#define PARAM_NO_USE_0 0

#define NULL_PTR 0
#define NULL_BUF 0

#define BUFSIZE_0 0


// 2002-9-24: Because I am fond of using "unsigned char" as default char type,
//while some compiler(including VC) does not support "unsigned char" as default
//"char" type, I have to explicitly define the two macros to deal with the
//consequent conversion.
# define _C(str) ((char*)(str))
# define _UC(str) ((unsigned char*)(str))

// Get the offset-value of `Member' in struct `Struct'
#if defined REQUIRE_offsetof && !defined(offsetof)
# define offsetof(Struct, Member) ( (int)& ((Struct*)0) -> Member )
#endif
// Note: VC++6 & ARM SDT have defined this in STDDEF.H

// Get the element quantity of an array
#define GetEleQuan(array) ( sizeof(array)/sizeof(array[0]) )
	// Note that GetEleQuan() "returns" a unsigned int value !
#define GetEleQuan_i(array) ((int)GetEleQuan(array))

// Get the Index into a 2D array by giving `y', `countx', and `x'
#define GI2D(y,countx,x) ((y)*(countx)+(x))
#define GI2D_L(y,countx,x) ((long)(y)*(countx)+(x))

# define SUCCESS_0 0 //for some lib-function return 0 on success
# define NOERROR_0 0
# define FAIL__1 -1

typedef int SorF;	// SorF means Success or Fail.
# define SUCCESS_1 1
# define FAIL_0 0

typedef int TorF;	// TorF means True or False
#if !(defined FALSE) && !(defined TRUE)
	// I think nobody will let TRUE=0 and FAIL=1
# define FALSE 0
# define TRUE 1
#endif 

typedef int YorN;
#if !(defined YES) && !(defined NO)
	// I think nobody will let YES=0 and NO=1
# define YES 1
# define NO 0
#endif

#define DELETE_P(p) do{ if(p){delete p; p=0;} } while(0)
#define DELETE_ARRAY(array) \
	do{ if(array){ delete []array; array=0;} }while(0)
#define DELETE_ARRAY_E DELETE_ARRAY
	// the trailing "_E" means "if Exists".

#define FREE_MALLOCED(p) \
	do{ if(p){free(p); p=0;} }while(0)
#define FREE_MALLOCED_E FREE_MALLOCED

#define CALL_IF_PRESENT(pFunc, ParamList) do{ if(pFunc) pFunc ParamList; }while(0)
	// call the function pointed to by `pFunc' if `pFunc' is not NULL.

#define ROUND2POWER(n, x) ( (n) & ~((x)-1) ) //down-round
	// x must be 2's power, e.g.
	// ROUND2POWER(8, 4) = ROUND2POWER(9, 4) = ROUND2POWER(10, 4) = ROUND2POWER(11, 4) = 8
	// ROUND2POWER(12, 4) = 12 

#define ROUND2POWER_U(n, x) ( (n) &= ~((x)-1) ) // _U means update.

#define UPROUND2POWER(n, x) ( (n)+(x)-1 & ~((x)-1) )
	// x must be 2's power, e.g.
	// UPROUND2POWER(8, 4) = 8
	// UPROUND2POWER(9, 4) = UPROUND2POWER(10, 4) = UPROUND2POWER(11, 4) = UPROUND2POWER(12, 4) = 12 

#define UPROUND2POWER_U(n, x) ( (n) = UPROUND2POWER(n,x) ) // _U means update.


#define OCC_DIVIDE(n, x) ( ((n)+(x)-1) / (x) ) // occupation divide, seeking for a better name

#define DOWN_ROUND(n, x) ( (n) / (x) * (x) )
#define UP_ROUND(n, x) ( OCC_DIVIDE(n, x) * (x) )

#define DOWN_ROUND_U(n, x) ( (n) = DOWN_ROUND(n,x) )
#define UP_ROUND_U(n, x) ( (n) = UP_ROUND(n,x) )


#define __lb__ {
#define __rb__ }


#ifdef __cplusplus
  #define ENUM_ENABLE_BITWISE_OR(enum_type) \
	inline enum_type operator|(enum_type a, enum_type b) \
	{ \
		return enum_type((int)a|b); \
	}
	// Using this macro to define an overloaded bitwise-or operator, so that
	// bitwise-or of two or more enum values will result in the same enum type.
	// Without this function, the result was an int.
	// NOTE: ENUM_ENABLE_BITWISE_OR should normally be used in global scope, not in a class definition.
#endif // #ifdef __cplusplus

#endif // #ifndef __CHJ_COMMDEFS_H


