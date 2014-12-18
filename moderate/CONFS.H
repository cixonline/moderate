#ifndef _HORUS_MODERATE_CONFS_H
#define _HORUS_MODERATE_CONFS_H



#include <time.h>



enum {
    CTI_REVISION=3,     //  revision level for topic information (.CTI) file
    MAX_TOPICS=256,     //  max number of topics (ok for all but norman :-)
    DESC_LENGTH=64      //  max length for conf/topic descriptions
};



#define DEFAULT_TOPIC_WARNING 10000L


#pragma warning(disable : 4341)
enum {
    TOPIC_NEWCONF=0x0001,
    TOPIC_NEW=0x0002,
    TOPIC_DELETE=0x0004,
    TOPIC_PRUNE=0x0008,
    TOPIC_FILES=0x0010,
    TOPIC_READONLY=0x0020,
    TOPIC_UPDATELIST=0x0040,
    TOPIC_UPDATESIZE=0x0080,
    TOPIC_UPDATEFLIST=0x0100,
    TOPIC_UPDATEPART=0x0200,
    TOPIC_REQUESTLIST=0x0400,
    TOPIC_REQUESTSIZE=0x0800,
    TOPIC_SIZEWARNING=0x1000,
    TOPIC_GETNOTE=0x2000,
    TOPIC_PUTNOTE=0x4000,
    TOPIC_UPDATEFDIR=0x8000,

    TOPIC_SAVEMASK=TOPIC_UPDATESIZE|TOPIC_UPDATEFLIST|TOPIC_UPDATEPART|
                   TOPIC_REQUESTSIZE|TOPIC_SIZEWARNING|TOPIC_UPDATEFDIR

};
#pragma warning(default : 4341)



typedef struct {
    //  Topic information structure
    int topicIndex;
    char topicName[15];
    HTOPIC topicHandle;
    WORD topicFlags;
    WORD topicOldFlags;
    time_t lastUpdate;
    WORD updateSelect;
    WORD updateDays;
    LONG topicSize;
    LONG topicMax;
    LONG topicWarn;
    time_t lastFlistUpdate;
    WORD flistSelect;
    WORD flistDays;
    char topicDescription[DESC_LENGTH];
    HOOB hoobSeedMessage;
} Topic, FAR *LPTopic;



typedef struct {
    WORD fileVersion;
    time_t lastUpdate;
    WORD updateSelect;
    WORD updateDays;
    WORD updateFlags;
    LONG topicWarn;
    time_t lastFlistUpdate;
    WORD flistSelect;
    WORD flistDays;
} TopicHeader, FAR *LPTopicHeader;



typedef struct {
    char szConfName[15];
    TopicHeader topicHeader;
    LPTopic topics[MAX_TOPICS];
    LPTopic currentTopic;
    int topicCount;
    int totalCount;
    BOOL changed;
} TopicInfo, FAR *LPTopicInfo;



typedef struct {
    FARPROC lpfnTopicNameProc;
    WNDPROC lpfnTopicNameOldProc;        //  original topic name edit control message handler
    WNDPROC lpfnTopicListProc;           //  topic list list box message handler
    HWND hwndTopicList;                  //  topic list list box window handle
} TopicSubclassData, FAR *LPTopicSubclassData;



typedef struct {
    TopicSubclassData topicSubclass;
    HCONF hConference;
    TopicInfo topicInfo;
    LPTopic lpTopic;
    LPTopic lpPrevTopic;
    BOOL bNewTopic;
    BOOL bFlistSet;
    BOOL bReadonlySet;
    Topic newTopic;
    BOOL bTopicFocus;
    BOOL bTopicChanged;
    BOOL bTopicSet;
    BOOL bInValidate;
    BOOL bCheckFlag;
} EditTopicData, FAR *LPEditTopicData;



void resetTopicInfo(LPTopicInfo lpTopicInfo);
void initTopicInfo(LPTopicInfo lpTopicInfo);
void freeTopicInfo(LPTopicInfo lpTopicInfo);
void deleteSeedMessages(LPTopicInfo lpTopicInfo, BOOL bAll);
void initTopic(LPTopic lpTopic, LPTopicHeader lpTopicHeader);
LPTopic allocateTopic(LPTopicInfo lpTopicInfo, LPTopic lpTopicAssign);
void removeTopic(LPTopicInfo lpTopicInfo, LPTopic lpTopic);
void saveTopic(HWND hdlg, LPTopic lpTopic);
LPTopic findTopic(LPTopicInfo lpTopicInfo, LPSTR lpTopicName, int nTopicStart);
void displayTopic(HWND hdlg, LPTopic lpTopic, BOOL bSetName);
LPTopic selectTopic(HWND hdlg, LPTopicInfo lpTopicInfo, int topicIndex, BOOL bSetName);
BOOL readTopicHeader(HSTREAM hStream, LPTopicHeader lpTopicHeader);
void writeTopicHeader(HSTREAM hStream, LPTopicHeader lpTopicHeader);
BOOL getTopicInfo(HWND hdlg, HCONF hConf, LPTopicInfo lpTopicInfo);
BOOL loadTopicInfo(HWND hdlg, HCONF hConf, LPTopicInfo lpTopicInfo);
void editSeedMessage(HWND hdlg, LPTopicInfo lpTopicInfo);
void writeSeedMessage(HWND hdlg, LPTopicInfo lpTopicInfo, BOOL bEdit);
BOOL saveTopicInfo(HWND hdlg, HCONF hConf, LPTopicInfo lpTopicInfo);
void drawTopicName(const DRAWITEMSTRUCT FAR * lpDis);
void editConfNote(HWND hdlg, LPTopicInfo lpTopicInfo, LPCSTR lpcName);

BOOL _EXPORT CALLBACK updateTopicProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);




#endif
