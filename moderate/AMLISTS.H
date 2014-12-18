#ifndef _HORUS_HCTOOLS_AMLISTS_H
#define _HORUS_HCTOOLS_AMLISTS_H



DECLARE_HANDLE32(HAMLIST);

enum {
    LEN_CONFDESCRIPTION=128,
    LEN_UNETDESCRIPTION=128,
    LEN_USERDESCRIPTION=128,
    LEN_UNETNAME=64
};

typedef struct {
    enum {CONF_OPEN, CONF_CLOSED, CONF_UNKNOWN} eCategory;
    char szConfName[LEN_CONFNAME];
    char szConfDescription[LEN_CONFDESCRIPTION];
} ConfListEntry, FAR *LPConfListEntry;

typedef struct {
    char szGroupName[LEN_UNETNAME];
    char szGroupDescription[LEN_UNETDESCRIPTION];
} UsenetListEntry, FAR *LPUsenetListEntry;

typedef struct {
    char szUserName[LEN_CIXNAME];
    char szUserDescription[LEN_USERDESCRIPTION];
} UserListEntry, FAR *LPUserListEntry;



HAMLIST FAR PASCAL _EXPORT OpenAmeolList(LPCSTR lpcName);
int FAR PASCAL _EXPORT ReadConferenceList(HAMLIST hStream, LPConfListEntry lpConfListEntry);
int FAR PASCAL _EXPORT ReadUsenetList(HAMLIST hStream, LPUsenetListEntry lpUsenetListEntry);
int FAR PASCAL _EXPORT ReadUserList(HAMLIST hStream, LPUserListEntry lpUserListEntry);
void FAR PASCAL _EXPORT CloseAmeolList(HAMLIST hStream);

#define OpenConferenceList()		OpenAmeolList("cix.dat")
#define OpenUsenetList()		OpenAmeolList("usenet.dat")
#define OpenUserList()			OpenAmeolList("users.dat")

#define CloseConferenceList(hStream)	CloseAmeolList(hStream)
#define CloseUsenetList(hStream)	CloseAmeolList(hStream)
#define CloseUserList(hStream)		CloseAmeolList(hStream)



#endif

