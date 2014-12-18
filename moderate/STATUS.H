#ifndef _HORUS_MODERATOR_STATUS_H
#define _HORUS_MODERATOR_STATUS_H



#if !defined(OK)
#define OK 0
#endif



enum {
    ERR_FILE_MISSING=1,
    ERR_FLIST_CHANGED,
    ERR_OPEN,
    ERR_WRITE,
    ERR_BAD_FLM,
    ERR_BAD_FLD
};



#endif
