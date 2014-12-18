#ifndef _HORUS_MODERATE_SORT_H
#define _HORUS_MODERATE_SORT_H



typedef struct {
    int sortDirection;
    int sortKey;
} SortData, FAR *LPSortData;



void sortFdir(LPPFdirEntry fdirHeader, int fdirCount);



#endif
