#ifndef _HORUS_MODERATOR_HELP_H
#define _HORUS_MODERATOR_HELP_H



#include "..\help\confs.h"
#include "..\help\create.h"
#include "..\help\topics.h"
#include "..\help\moderate.h"
#include "..\help\flist.h"
#include "..\help\settings.h"


enum {
    DID_ABOUT,
    DID_CONF_NOTE,
    DID_CONFIGURE,
    DID_CREATE_CONF,
    DID_DELETE_FLIST,
    DID_DELETE_TOPIC,
    DID_DLL_INFO,
    DID_DUPLICATE,
    DID_EXPORT,
    DID_FILE_DESCRIPTION,
    DID_FILE_DIRECTORY,
    DID_FLIST,
    DID_FLIST_ERROR,
    DID_FLIST_MISSING,
    DID_IMPORT_NOTIFICATION,
    DID_IMPORT_SELECT,
    DID_INSERT_FDIR,
    DID_INSERT_FILEPOOL,
    DID_MESSAGE_EDIT,
    DID_NOTIFY_SELECT,
    DID_PRUNE_TOPIC,
    DID_RENAME,
    DID_SELECT_TOPIC,
    DID_SORT_FLIST,
    DID_TOPIC_MAINTENANCE,
    DID_UPDATE_LISTS,
    DID_UPDATE_TOPIC_DATA,
    DID_VALIDATE_DOWNLOAD,
    DID_VALIDATE_UPDATE
};



LPCSTR balloonHelp(int nDialog, int nID);



#endif
