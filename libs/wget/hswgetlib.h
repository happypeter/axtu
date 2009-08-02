#ifndef __MYLIB_H__
#define __MYLIB_H__

#include "wget.h"
#include "utils.h"
#include "init.h"
#include "retr.h"
#include "recur.h"
#include "host.h"
#include "url.h"
#include "progress.h"
#include "convert.h"

extern "C" { uerr_t retrieve_url PARAMS ((const char *, char **, char **, const char *, int *));      }
extern "C" { void set_progress_implementation PARAMS ((const char *));        }
extern "C" { void initialize PARAMS ((void)); }
extern "C" { void SetGetFileCallBack( GetFileCallBackFunc callBackFunc );  }
extern "C" { void setoptval PARAMS ((const char *, const char *, const char *));        }


#endif
