#ifndef _HORUS_DEBUGDEF_H
#define _HORUS_DEBUGDEF_H



int _alert(UINT uLevel, LPCSTR format,...);



#if defined(_DEBUG)
    extern UINT _alertLevel;

    #define _ALERT _alert

    #define _GCHECK(uLevel, szTag) \
        (_alertLevel>=(uLevel) ? gcheckall((szTag), NULL) : TRUE)

#else
    #define _ALERT 1 ? 0 : _alert
    #define _GCHECK(uLevel, szTag) TRUE
#endif



#endif
