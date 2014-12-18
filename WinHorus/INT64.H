#ifndef _HORUS_INT64_H
#define _HORUS_INT64_H



//  int64.h - 64-bit integer definitions
//            © 1995 Pete Jordan, Horus Communication
//            Please see the accompanying file "copyleft.txt" for details
//            of your licence to use and distribute this program.



typedef struct {
    DWORD dwHigh;
    DWORD dwLow;
} QWORD, FAR *LPQWORD;



#define zero64(qw) ((qw).dwHigh=(qw).dwLow=0L)
#define high64(qw) ((qw).dwHigh=(qw).dwLow=0xffffffffL)
#define inc64(qw) (++(qw).dwLow==0L ? (qw).dwHigh++ : 0)
#define dec64(qw) (--(qw).dwLow==0xffffffffL ? (qw).dwHigh-- : 0)

#define cmp64(qw1, qw2) \
    ((qw1).dwHigh<(qw2).dwHigh ? -1 : \
     (qw1).dwHigh>(qw2).dwHigh ? 1 : \
     (qw1).dwLow<(qw2).dwLow ? -1 : \
     (qw1).dwLow>(qw2).dwLow ? 1 : 0)



#endif