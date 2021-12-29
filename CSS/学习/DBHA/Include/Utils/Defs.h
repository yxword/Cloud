//
//  "$Id: Defs.h 16861 2010-12-03 07:41:03Z wang_haifeng $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __DAHUA_UTILS_DEFS_H__
#define __DAHUA_UTILS_DEFS_H__


// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef UTILS_DLL_BUILD
#define  UTILS_API _declspec(dllexport)
#elif defined UTILS_DLL_USE
#define  UTILS_API _declspec(dllimport)
#else
#define UTILS_API
#endif

#else

#define UTILS_API

#endif


//////////////////////////////////////////////////////////////////////////
// use the unified 'DEBUG' macro
#if (!defined(NDEBUG)) && !defined(DEBUG)
#	define DEBUG
#endif


#endif // __DAHUA_UTILS_DEFS_H__
