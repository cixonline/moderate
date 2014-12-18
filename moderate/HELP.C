#include <windows.h>
#include "ameolapi.h"
#include "moderate.h"
#include "strings.h"
#include "winhorus.h"
#include "globals.h"
#include "help.h"



LPCSTR balloonHelp(int nDialog, int nID)
{
    int nHelpID=0;

    switch (nDialog) {

    case DID_ABOUT:
        switch (nID) {

        case CID_OK:                    nHelpID=SID_OK;                      break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;
        case CID_DLL_INFO:              nHelpID=SID_DLL_INFO;                break;
        case CID_CONFIGURE:             nHelpID=SID_CONFIGURE;               break;
        case CID_ABOUT_CONF:            nHelpID=SID_ABOUT_CONF;              break;
        case CID_ABOUT_TOPIC:           nHelpID=SID_ABOUT_TOPIC;             break;
        case CID_ABOUT_FLIST:           nHelpID=SID_ABOUT_FLIST;             break;

        }

        break;

    case DID_CONF_NOTE:
        switch (nID) {

	case CID_UNDO:                  nHelpID=SID_EDIT_UNDO;               break;
	case CID_DOWNLOAD:              nHelpID=SID_REFRESH_CONF_NOTE;       break;
	case CID_OK:                    nHelpID=SID_OK;                      break;
	case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
	case IDD_HELP:                  nHelpID=SID_HELP;                    break;

	}

	break;

    case DID_CONFIGURE:
	switch (nID) {

	case CID_DOWNLOAD_DIR:          nHelpID=SID_DOWNLOAD_DIR;            break;
	case CID_DOWNLOAD_BROWSE:       nHelpID=SID_DOWNLOAD_BROWSE;         break;
	case CID_UPLOAD_DIR:            nHelpID=SID_UPLOAD_DIR;              break;
	case CID_UPLOAD_BROWSE:         nHelpID=SID_UPLOAD_BROWSE;           break;
	case CID_DATE_FORMAT:           nHelpID=SID_DATE_FORMAT;             break;
	case CID_MENU_PREFIX:           nHelpID=SID_MENU_PREFIX;             break;
	case CID_EDIT_SEED:             nHelpID=SID_CFG_EDIT_SEED;           break;
	case CID_TOPICSIZE_WARNING:     nHelpID=SID_TOPICSIZE_WARNING;       break;
	case CID_LOCAL_TOPIC_DELETE:    nHelpID=SID_LOCAL_TOPIC_DELETE;      break;

	case CID_FLIST_SORT_NAME:       nHelpID=SID_FLIST_SORT_NAME;         break;
	case CID_FLIST_SORT_DATE:       nHelpID=SID_FLIST_SORT_DATE;         break;
	case CID_FLIST_SORT_SIZE:       nHelpID=SID_FLIST_SORT_SIZE;         break;
	case CID_FLIST_SORT_ASCENDING:  nHelpID=SID_FLIST_SORT_ASCENDING;    break;
	case CID_FLIST_SORT_DESCENDING: nHelpID=SID_FLIST_SORT_DESCENDING;   break;
	case CID_AUTO_NOTIFY:           nHelpID=SID_AUTO_NOTIFY;             break;
	case CID_NOTIFY_SIGNATURE:      nHelpID=SID_NOTIFY_SIGNATURE;        break;
	case CID_NOTIFY_MULTIPLE:       nHelpID=SID_CFG_NOTIFY_MULTIPLE;     break;
	case CID_NOTIFY_SIZE:           nHelpID=SID_NOTIFY_SIZE;             break;
	case CID_NOTIFY_UPPERCASE:      nHelpID=SID_NOTIFY_UPPERCASE;        break;
	case CID_CONFIRM_FLIST:         nHelpID=SID_CFG_CONFIRM_FLIST;       break;
	case CID_CONFIRM_FDIR:          nHelpID=SID_CFG_CONFIRM_FDIR;        break;
	case CID_LEAVE_FDIR:            nHelpID=SID_CFG_LEAVE_FDIR;          break;
	case CID_DELETE_ALL_FDIR:       nHelpID=SID_CFG_DELETE_ALL_FDIR;     break;

	case CID_ORPHANS:               nHelpID=SID_CFG_ORPHANS;             break;
	case CID_IN_FLIST:              nHelpID=SID_CFG_IN_FLIST;            break;
	case CID_ALL_FILES:             nHelpID=SID_CFG_ALL_FILES;           break;
	case CID_SORT_NAME:             nHelpID=SID_FDIR_SORT_NAME;          break;
	case CID_SORT_DATE:             nHelpID=SID_FDIR_SORT_DATE;          break;
	case CID_SORT_SIZE:             nHelpID=SID_FDIR_SORT_SIZE;          break;
	case CID_SORT_ASCENDING:        nHelpID=SID_FDIR_SORT_ASCENDING;     break;
	case CID_SORT_DESCENDING:       nHelpID=SID_FDIR_SORT_DESCENDING;    break;
	case CID_CONFIRM_ALL_FDIR:      nHelpID=SID_CFG_CONFIRM_ALL_FDIR;    break;
	case CID_CONFIRM_FLIST_FDIR:    nHelpID=SID_CFG_CONFIRM_FLIST_FDIR;  break;
	case CID_CONFIRM_NONE_FDIR:     nHelpID=SID_CFG_CONFIRM_NONE_FDIR;   break;

	case CID_OK:                    nHelpID=SID_OK;                      break;
	case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
	case IDD_HELP:                  nHelpID=SID_HELP;                    break;

	}

        break;

    case DID_CREATE_CONF:
        switch (nID) {

        case CID_CONF_NAME:             nHelpID=SID_CREATE_CONF_NAME;        break;
        case CID_CONF_DESCRIPTION:      nHelpID=SID_CONF_DESCRIPTION;        break;
        case CID_CONF_OPEN:             nHelpID=SID_CONF_OPEN;               break;
        case CID_CONF_CLOSED:           nHelpID=SID_CONF_CLOSED;             break;
        case CID_CONF_CONFIDENTIAL:     nHelpID=SID_CONF_CONFIDENTIAL;       break;
        case CID_TOPIC_NAME:            nHelpID=SID_CREATE_TOPIC_NAME;       break;
	case CID_TOPIC:                 nHelpID=SID_CREATE_TOPIC_LIST;       break;
        case CID_TOPIC_DESCRIPTION:     nHelpID=SID_TOPIC_DESCRIPTION;       break;
        case CID_ADD:                   nHelpID=SID_TOPIC_ADD;               break;
        case CID_DELETE:                nHelpID=SID_CREATE_DELETE;           break;
        case CID_CONF_NOTE:             nHelpID=SID_CREATE_CONF_NOTE;        break;
        case CID_EDIT_SEED:             nHelpID=SID_EDIT_SEED;               break;
        case CID_TOPIC_FLIST:           nHelpID=SID_CREATE_TOPIC_FLIST;      break;
        case CID_TOPIC_READONLY:        nHelpID=SID_CREATE_TOPIC_READONLY;   break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_DELETE_FLIST:
        switch (nID) {

        case CID_CONFIRM_FLIST:         nHelpID=SID_CONFIRM_FLIST;           break;
	case CID_CONFIRM_FDIR:          nHelpID=SID_CONFIRM_FDIR;            break;
        case CID_LEAVE_FDIR:            nHelpID=SID_LEAVE_FDIR;              break;
        case CID_DELETE_ALL_FDIR:       nHelpID=SID_DELETE_ALL_FDIR;         break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

	break;

    case DID_DELETE_TOPIC:
        switch (nID) {

        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;
        case CID_DELETE:                nHelpID=SID_CONFIRM_DELETE_TOPIC;    break;

        }

        break;

    case DID_DLL_INFO:
        switch (nID) {

        case CID_OK:                    nHelpID=SID_OK;                      break;

        }

        break;

    case DID_DUPLICATE:
        switch (nID) {

        case CID_DUP_CANCEL:            nHelpID=SID_DUP_CANCEL;              break;
        case CID_DUP_REPLACE:           nHelpID=SID_DUP_REPLACE;             break;
        case CID_DUP_DUPLICATE:         nHelpID=SID_DUP_DUPLICATE;           break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
	case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_EXPORT:
        switch (nID) {

        case CID_LOCAL:                 nHelpID=SID_EXPORT_LOCAL;            break;
        case CID_EXPORT:                nHelpID=SID_EXPORT_EXPORT;           break;
        case CID_TRANSFORM:             nHelpID=SID_EXPORT_TRANSFORM;        break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_FILE_DESCRIPTION:
        switch (nID) {

        case CID_DESCRIPTION:           nHelpID=SID_DESCRIPTION;             break;
        case CID_COMMENT_TEXT:          nHelpID=SID_COMMENT_TEXT;            break;
        case CID_TABBED_TEXT:           nHelpID=SID_COMMENT_TEXT;            break;
        case CID_INSERT_DATE:           nHelpID=SID_INSERT_DATE;             break;
        case CID_FILE_DATE:             nHelpID=SID_FILE_DATE;               break;
        case CID_PREVIOUS:              nHelpID=SID_PREVIOUS;                break;
        case CID_NEXT:                  nHelpID=SID_NEXT;                    break;
	case CID_UNDO:                  nHelpID=SID_UNDO;                    break;
	case IDD_HELP:                  nHelpID=SID_HELP;                    break;
	case CID_OK:                    nHelpID=SID_DONE_ALL;                break;

	}

	break;

    case DID_FILE_DIRECTORY:
	switch (nID) {

	case CID_OK:                    nHelpID=SID_OK;                      break;
	case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
	case IDD_HELP:                  nHelpID=SID_HELP;                    break;
	case CID_CHANGE_TOPIC:          nHelpID=SID_CHANGE_TOPIC;            break;
	case CID_DOWNLOAD_LISTS:        nHelpID=SID_DOWNLOAD_LISTS;          break;
	case CID_INSERT:                nHelpID=SID_INSERT;                  break;
	case CID_APPEND:                nHelpID=SID_APPEND;                  break;
	case CID_UPDATE_FILE:           nHelpID=SID_FDIR_UPDATE_FILE;        break;
	case CID_ACTION:                nHelpID=SID_FDIR_ACTION;             break;
	case CID_SORT:                  nHelpID=SID_FDIR_SORT;               break;
	case CID_ORPHANS:               nHelpID=SID_ORPHANS;                 break;
	case CID_IN_FLIST:              nHelpID=SID_IN_FLIST;                break;
	case CID_ALL_FILES:             nHelpID=SID_ALL_FILES;               break;

	}

	break;

    case DID_FLIST:
	switch (nID) {

	case CID_OK:                    nHelpID=SID_OK;                      break;
	case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
	case IDD_HELP:                  nHelpID=SID_HELP;                    break;
	case CID_CHANGE_TOPIC:          nHelpID=SID_CHANGE_TOPIC;            break;
	case CID_DOWNLOAD_LISTS:        nHelpID=SID_DOWNLOAD_LISTS;          break;
	case CID_INSERT:                nHelpID=SID_INSERT;                  break;
	case CID_APPEND:                nHelpID=SID_APPEND;                  break;
	case CID_UPDATE_FILE:           nHelpID=SID_UPDATE_FILE;             break;
	case CID_EDIT:                  nHelpID=SID_EDIT;                    break;
	case CID_IMPORT:                nHelpID=SID_IMPORT;                  break;
	case CID_MOVE_UP:               nHelpID=SID_MOVE_UP;                 break;
	case CID_MOVE_DOWN:             nHelpID=SID_MOVE_DOWN;               break;
	case CID_ACTION:                nHelpID=SID_FLIST_ACTION;            break;
	case CID_SORT:                  nHelpID=SID_SORT;                    break;
	case CID_UPLOAD_FLIST:          nHelpID=SID_UPLOAD_FLIST;            break;

	}

	break;

    case DID_FLIST_ERROR:
	switch (nID) {

        case CID_ERR_FILE:              nHelpID=SID_ERR_FILE;                break;
        case CID_ERR_MEMO:              nHelpID=SID_ERR_MEMO;                break;
        case CID_ERR_COMMENT:           nHelpID=SID_ERR_COMMENT;             break;
        case CID_ERR_INDENTED:          nHelpID=SID_ERR_INDENTED;            break;
        case CID_ERR_DISCARD:           nHelpID=SID_ERR_DISCARD;             break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_ABORT:                 nHelpID=SID_ERR_ABORT;               break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_FLIST_MISSING:
        switch (nID) {

        case CID_CHANGE_TOPIC:          nHelpID=SID_CHANGE_TOPIC;            break;
        case CID_CREATE_FLIST:          nHelpID=SID_CREATE_FLIST;            break;
        case CID_ASSUME:                nHelpID=SID_ASSUME;                  break;
	case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break; 

        }

        break;

    case DID_IMPORT_NOTIFICATION:
    case DID_IMPORT_SELECT:
        break;

    case DID_INSERT_FDIR:
        switch (nID) {

        case CID_OK:                    nHelpID=SID_INSERT_OK;               break;
        case CID_CANCEL:                nHelpID=SID_INSERT_CANCEL;           break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;
        case CID_SORT:                  nHelpID=SID_INSERT_SORT;             break;
        case CID_UPDATE:                nHelpID=SID_INSERT_UPDATE;           break;

        }

        break;

    case DID_INSERT_FILEPOOL:
        switch (nID) {

        case CID_FILENAME:              nHelpID=SID_FILENAME;                break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_MESSAGE_EDIT:
        switch (nID) {

        case CID_IMPORT:                nHelpID=SID_NOTIFY_IMPORT;           break;
        case CID_UNDO:                  nHelpID=SID_EDIT_UNDO;               break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;

        }

        break;

    case DID_NOTIFY_SELECT:
        switch (nID) {

        case CID_NOTIFY_SINGLE:         nHelpID=SID_NOTIFY_SINGLE;           break;
        case CID_NOTIFY_MULTIPLE:       nHelpID=SID_NOTIFY_MULTIPLE;         break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_PRUNE_TOPIC:
        switch (nID) {

        case CID_PRUNE:                 nHelpID=SID_CONFIRM_PRUNE;           break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_RENAME:
        switch (nID) {

        case CID_NEWNAME:               nHelpID=SID_NEWNAME;                 break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case CID_CANCEL_ALL:            nHelpID=SID_CANCEL_ALL;              break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_SELECT_TOPIC:
        switch (nID) {

        case CID_CONFERENCE:            nHelpID=SID_CONFERENCE;              break;
        case CID_TOPIC:                 nHelpID=SID_TOPIC;                   break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_SORT_FLIST:
        switch (nID) {

        case CID_SORT_NAME:             nHelpID=SID_SORT_NAME;               break;
        case CID_SORT_DATE:             nHelpID=SID_SORT_DATE;               break;
        case CID_SORT_SIZE:             nHelpID=SID_SORT_SIZE;               break;
        case CID_SORT_ASCENDING:        nHelpID=SID_SORT_ASCENDING;          break;
        case CID_OK:                    nHelpID=SID_SORT_OK;                 break;
        case CID_CANCEL:                nHelpID=SID_SORT_CANCEL;             break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_TOPIC_MAINTENANCE:
        switch (nID) {

        case CID_CONFERENCE:            nHelpID=SID_TOPIC_CONFERENCE;        break;
        case CID_TOPIC_NAME:            nHelpID=SID_TOPIC_TOPIC_NAME;        break;
        case CID_TOPIC:                 nHelpID=SID_TOPIC_TOPIC_LIST;        break;
        case CID_TOPIC_DESCRIPTION:     nHelpID=SID_TOPIC_DESCRIPTION;       break;
        case CID_ADD:                   nHelpID=SID_TOPIC_ADD;               break;
        case CID_DELETE:                nHelpID=SID_TOPIC_DELETE;            break;
        case CID_PRUNE:                 nHelpID=SID_TOPIC_PRUNE;             break;
        case CID_UPDATE:                nHelpID=SID_TOPIC_UPDATE;            break;
        case CID_CONF_NOTE:             nHelpID=SID_TOPIC_CONF_NOTE;         break;
        case CID_EDIT_SEED:             nHelpID=SID_EDIT_SEED;               break;
        case CID_TOPIC_FLIST:           nHelpID=SID_TOPIC_TOPIC_FLIST;       break;
        case CID_TOPIC_READONLY:        nHelpID=SID_TOPIC_TOPIC_READONLY;    break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    case DID_UPDATE_LISTS:
        switch (nID) {

        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;
        case CID_FLM_UPDATE:            nHelpID=SID_FLM_UPDATE;              break;
        case CID_FLD_UPDATE:            nHelpID=SID_FLD_UPDATE;              break;
        case CID_FLS_UPDATE:            nHelpID=SID_FLS_UPDATE;              break;

        }

        break;

    case DID_UPDATE_TOPIC_DATA:
        switch (nID) {

        case CID_TOPIC:                 nHelpID=SID_UPDATE_TOPIC_NAME;       break;
        case CID_UPDATE_TOPICS:         nHelpID=SID_UPDATE_TOPICS;           break;
	case CID_UPDATE_NEXT:           nHelpID=SID_UPDATE_NEXT;             break;
	case CID_TOPICSIZE_WARNING:     nHelpID=SID_TOPICSIZE_WARNING;       break;
	case CID_TOPICSIZE_ENABLE:      nHelpID=SID_TOPICSIZE_ENABLE;        break;
        case CID_TOPICSIZE_THRESHOLD:   nHelpID=SID_TOPICSIZE_THRESHOLD;     break;
        case CID_UPDATE_MANUAL:         nHelpID=SID_UPDATE_MANUAL;           break;
        case CID_UPDATE_PERIODIC:       nHelpID=SID_UPDATE_PERIODIC;         break;
        case CID_UPDATE_ALWAYS:         nHelpID=SID_UPDATE_ALWAYS;           break;
	case CID_UPDATE_DAYS:           nHelpID=SID_UPDATE_DAYS;             break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;
        case CID_RESET:                 nHelpID=SID_RESET;                   break;

        }

        break;

    case DID_VALIDATE_DOWNLOAD:
        switch (nID) {

        case CID_NEWNAME:               nHelpID=SID_VALIDATE_NEWNAME;        break;
        case CID_OK:                    nHelpID=SID_OK;                      break;
        case CID_CANCEL:                nHelpID=SID_CANCEL;                  break;
        case IDD_HELP:                  nHelpID=SID_HELP;                    break;

        }

        break;

    }

    if (nHelpID) {
        static char buff[256];

        LoadString(lpGlobals->hInst, nHelpID, buff, 255);
        return((LPCSTR)buff);
    } else
        return(NULL);

}

