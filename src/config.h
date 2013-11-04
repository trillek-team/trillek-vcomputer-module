#ifndef ___CONFIG_H__
#define ___CONFIG_H__

//This file is used to have a VC++ compatibility

#if defined(_MSC_VER)
//VC++ C compiler support : C89 thanks microsoft !
#define snprintf _snprintf 
//Get bored of theses warnings
#pragma warning(disable : 4996)
#pragma warning(disable : 4333)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)
#pragma warning(disable : 4018)
#endif


#endif