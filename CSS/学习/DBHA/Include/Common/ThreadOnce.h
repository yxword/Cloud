//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_PTHREAD_H__
#define __INCLUDE_DAHUA_EFS_PTHREAD_H__

#ifdef WIN32

struct efs_pthread_once_t_
{
  int          done;        /* indicates if user function has been executed */
  void *       lock;
  int          reserved1;
  int          reserved2;
};

typedef struct efs_pthread_once_t_ efs_pthread_once_t;

/*
 * ====================
 * ====================
 * Once Key
 * ====================
 * ====================
 */
#define EFS_PTHREAD_ONCE_INIT       { 0, 0, 0, 0}


int efs_pthread_once (efs_pthread_once_t * once_control,
                          void ( *init_routine) (void));
#else

#include <pthread.h>

#define efs_pthread_once_t pthread_once_t
#define EFS_PTHREAD_ONCE_INIT PTHREAD_ONCE_INIT
#define efs_pthread_once pthread_once

#endif

#endif //__INCLUDE_DAHUA_EFS_PTHREAD_H__
