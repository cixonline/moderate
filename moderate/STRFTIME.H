#ifndef OUR_STRFTIME
#define OUR_STRFTIME

#ifndef WIN32

size_t __cdecl strftime (char *string,size_t maxsize,const char *format,const struct tm *timeptr);

#endif
#endif
	