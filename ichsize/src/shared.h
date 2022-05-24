#ifndef _shared_h_
#define _shared_h_

#ifdef __cplusplus

#ifndef ARRAYSIZE

// define ARRAYSIZE macro, borrowed from Visual C++ winnt.h, will used by Linux

template <typename T, size_t N>
char (*
	RtlpNumberOf( T (& func_param)[N] )
)[N];

#define ARRAYSIZE(A)    ( sizeof(*RtlpNumberOf(A)) )

#endif // not defined ARRAYSIZE

#endif // has __cplusplus

#endif
