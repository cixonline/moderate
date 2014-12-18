#include <windows.h>

#ifndef WIN32

//#include <cruntime.h>
//#include <internal.h>
//#include <mtdll.h>
#include <time.h>
#include <locale.h>
//#include <setlocal.h>
#include <ctype.h>
#include <string.h>

/* Prototypes for local routines */
static void __cdecl _expandtime (char specifier, const struct tm *tmptr, char **out, size_t *count);
static void __cdecl _store_str (char *in, char **out, size_t *count);
static void __cdecl _store_num (int num, int digits, char **out, size_t *count);
static void __cdecl _store_number (int num, char **out, size_t *count);
static void __cdecl _store_winword (const char *format, const struct tm *tmptr, char **out, size_t *count);


/* LC_TIME data for local "C" */

struct __lc_time_data {
        char *wday_abbr[7];
        char *wday[7];
        char *month_abbr[12];
        char *month[12];
        char *ampm[2];
        char *ww_sdatefmt;
        char *ww_ldatefmt;
        char *ww_timefmt;
        };

struct __lc_time_data __lc_time_c = {

        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},

        {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
                "Friday", "Saturday", },

        {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
                "Sep", "Oct", "Nov", "Dec"},

        {"January", "February", "March", "April", "May", "June",
                "July", "August", "September", "October",
                "November", "December"},

        {"AM", "PM"}

        , { "M/d/yy" }
        , { "dddd, MMMM dd, yyyy" }
        , { "H:mm:ss" }
        };

/* Pointer to the current LC_TIME data structure. */

struct __lc_time_data *__lc_time_curr = &__lc_time_c;

/* Flags */
unsigned _alternate_form = 0;
unsigned _no_lead_zeros = 0;

/***
*size_t strftime(string, maxsize, format, timeptr) - Format a time string
*
*Purpose:
*       Place characters into the user's output buffer expanding time
*       format directives as described in the user's control string.
*       Use the supplied 'tm' structure for time data when expanding
*       the format directives.
*       [ANSI]
*
*Entry:
*       char *string = pointer to output string
*       size_t maxsize = max length of string
*       const char *format = format control string
*       const struct tm *timeptr = pointer to tb data structure
*
*Exit:
*       !0 = If the total number of resulting characters including the
*       terminating null is not more than 'maxsize', then return the
*       number of chars placed in the 'string' array (not including the
*       null terminator).
*
*       0 = Otherwise, return 0 and the contents of the string are
*       indeterminate.
*
*Exceptions:
*
*******************************************************************************/

size_t __cdecl strftime (
        char *string,
        size_t maxsize,
        const char *format,
        const struct tm *timeptr
        )
{

        size_t left;                    /* space left in output string */


        /* Copy maxsize into temp. */
        left = maxsize;

        /* Copy the input string to the output string expanding the format
        designations appropriately.  Stop copying when one of the following
        is true: (1) we hit a null char in the input stream, or (2) there's
        no room left in the output stream. */

        while (left > 0)
        {
                switch(*format)
                {

                case('\0'):

                        /* end of format input string */
                        goto done;

                case('%'):

                        /* Format directive.  Take appropriate action based
                        on format control character. */

                        format++;                       /* skip over % char */

                        /* process flags */
                        _alternate_form = 0;
                        if (*format == '#')
                        {
                                _alternate_form = 1;
                                format++;
                        }
                        _expandtime(*format, timeptr, &string, &left);
                        format++;                       /* skip format char */
                        break;


                default:

                        /* store character, bump pointers, dec the char count */
                        if (isleadbyte((int)(*format)) && left > 1)
                        {
                                *string++ = *format++;
                                left--;
                        }
                        *string++ = *format++;
                        left--;
                        break;
                }
        }


        /* All done.  See if we terminated because we hit a null char or because
        we ran out of space */

        done:

        if (left > 0) {

                /* Store a terminating null char and return the number of chars
                we stored in the output string. */

                *string = '\0';
                return(maxsize-left);
        }

        else
                return(0);

}


/***
*_expandtime() - Expand the conversion specifier
*
*Purpose:
*       Expand the given strftime conversion specifier using the time struct
*       and store it in the supplied buffer.
*
*       The expansion is locale-dependent.
*
*       *** For internal use with strftime() only ***
*
*Entry:
*       char specifier = strftime conversion specifier to expand
*       const struct tm *tmptr = pointer to time/date structure
*       char **string = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*
*Exit:
*       none
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl _expandtime (
        char specifier,
        const struct tm *timeptr,
        char **string,
        size_t *left
        )
{
        struct __lc_time_data *lc_time; /* lc_time data pointer */
        unsigned temp;                  /* temps */
        int wdaytemp;

        /* Get a copy of the current __lc_time_data pointer.  This
        should prevent the necessity of locking/unlocking in mthread
        code (if we can guarantee that the various __lc_time data
        structures are always in the same segment). contents of time
        strings structure can now change, so thus we do use locking */

        lc_time = __lc_time_curr;

        switch(specifier) {             /* switch on specifier */

                case('a'):              /* abbreviated weekday name */
                        _store_str((char *)(lc_time->wday_abbr[timeptr->tm_wday]),
                                 string, left);
                        break;

                case('A'):              /* full weekday name */
                        _store_str((char *)(lc_time->wday[timeptr->tm_wday]),
                                 string, left);
                        break;

                case('b'):              /* abbreviated month name */
                        _store_str((char *)(lc_time->month_abbr[timeptr->tm_mon]),
                                 string, left);
                        break;

                case('B'):              /* full month name */
                        _store_str((char *)(lc_time->month[timeptr->tm_mon]),
                                 string, left);
                        break;

                case('c'):              /* date and time display */
                        if (_alternate_form)
                        {
                                _alternate_form = FALSE;
                                _store_winword(lc_time->ww_ldatefmt, timeptr, string, left);
                                if (*left == 0)
                                        return;
                                *(*string)++=' ';
                                (*left)--;
                                _store_winword(lc_time->ww_timefmt, timeptr, string, left);
                        }
                        else {
                                _store_winword(lc_time->ww_sdatefmt, timeptr, string, left);
                                if (*left == 0)
                                        return;
                                *(*string)++=' ';
                                (*left)--;
                                _store_winword(lc_time->ww_timefmt, timeptr, string, left);
                        }
                        break;

                case('d'):              /* mday in decimal (01-31) */
                        _no_lead_zeros = _alternate_form;
                        _store_num(timeptr->tm_mday, 2, string, left);
                        break;

                case('H'):              /* 24-hour decimal (00-23) */
                        _no_lead_zeros = _alternate_form;
                        _store_num(timeptr->tm_hour, 2, string, left);
                        break;

                case('I'):              /* 12-hour decimal (01-12) */
                        _no_lead_zeros = _alternate_form;
                        if (!(temp = timeptr->tm_hour%12))
                                temp=12;
                        _store_num(temp, 2, string, left);
                        break;

                case('j'):              /* yday in decimal (001-366) */
                        _no_lead_zeros = _alternate_form;
                        _store_num(timeptr->tm_yday+1, 3, string, left);
                        break;

                case('m'):              /* month in decimal (01-12) */
                        _no_lead_zeros = _alternate_form;
                        _store_num(timeptr->tm_mon+1, 2, string, left);
                        break;

                case('M'):              /* minute in decimal (00-59) */
                        _no_lead_zeros = _alternate_form;
                        _store_num(timeptr->tm_min, 2, string, left);
                        break;

                case('p'):              /* AM/PM designation */
                        if (timeptr->tm_hour <= 11)
                            _store_str((char *)(lc_time->ampm[0]), string, left);
                        else
                            _store_str((char *)(lc_time->ampm[1]), string, left);
                        break;

                case('S'):              /* secs in decimal (00-59) */
                        _no_lead_zeros = _alternate_form;
                        _store_num(timeptr->tm_sec, 2, string, left);
                        break;

                case('U'):              /* sunday week number (00-53) */
                        _no_lead_zeros = _alternate_form;
                        wdaytemp = timeptr->tm_wday;
                        goto weeknum;   /* join common code */

                case('w'):              /* week day in decimal (0-6) */
                        _no_lead_zeros = _alternate_form;
                        _store_num(timeptr->tm_wday, 1, string, left);
                        break;

                case('W'):              /* monday week number (00-53) */
                        _no_lead_zeros = _alternate_form;
                        if (timeptr->tm_wday == 0)  /* monday based */
                                wdaytemp = 6;
                        else
                                wdaytemp = timeptr->tm_wday-1;
                weeknum:
                        if (timeptr->tm_yday < wdaytemp)
                                temp=0;
                        else {
                                temp = timeptr->tm_yday/7;
                                if ((timeptr->tm_yday%7) >= wdaytemp)
                                        temp++;
                                }
                        _store_num(temp, 2, string, left);
                        break;

                case('x'):              /* date display */
                        if (_alternate_form)
                        {
                                _alternate_form = FALSE;
                                _store_winword(lc_time->ww_ldatefmt, timeptr, string, left);
                        }
                        else
                        {
                                _store_winword(lc_time->ww_sdatefmt, timeptr, string, left);
                        }
                        break;

                case('X'):              /* time display */
                        _alternate_form = FALSE;
                        _store_winword(lc_time->ww_timefmt, timeptr, string, left);
                        break;

                case('y'):              /* year w/o century (00-99) */
                        _no_lead_zeros = _alternate_form;
                        temp = timeptr->tm_year%100;
                        _store_num(temp, 2, string, left);
                        break;

                case('Y'):              /* year w/ century */
                        _no_lead_zeros = _alternate_form;
                        temp = (((timeptr->tm_year/100)+19)*100) +
                               (timeptr->tm_year%100);
                        _store_num(temp, 4, string, left);
                        break;

                case('%'):              /* percent sign */
                        *(*string)++ = '%';
                        (*left)--;
                        break;

                default:                /* unknown format directive */
                        /* ignore the directive and continue */
                        /* [ANSI: Behavior is undefined.]    */
                        break;

        }       /* end % switch */
}


/***
*_store_str() - Copy a time string
*
*Purpose:
*       Copy the supplied time string into the output string until
*       (1) we hit a null in the time string, or (2) the given count
*       goes to 0.
*
*       *** For internal use with strftime() only ***
*
*Entry:
*       char *in = pointer to null terminated time string
*       char **out = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*
*Exit:
*       none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_str (
        char *in,
        char **out,
        size_t *count
        )
{

while ((*count != 0) && (*in != '\0')) {
        *(*out)++ = *in++;
        (*count)--;
        }
}


/***
*_store_num() - Convert a number to ascii and copy it
*
*Purpose:
*       Convert the supplied number to decimal and store
*       in the output buffer.  Update both the count and
*       buffer pointers.
*
*       *** For internal use with strftime() only ***
*
*Entry:
*       int num = pointer to integer value
*       int digits = # of ascii digits to put into string
*       char **out = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*
*Exit:
*       none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_num (
        int num,
        int digits,
        char **out,
        size_t *count
        )
{
int temp=0;

        if (_no_lead_zeros) {
                _store_number (num, out, count);
                return;
        }

if ((size_t)digits < *count)  {
        for (digits--; (digits+1); digits--) {
                (*out)[digits] = (char)('0' + num % 10);
                num /= 10;
                temp++;
                }
        *out += temp;
        *count -= temp;
        }
else
        *count = 0;
}

/***
*_store_number() - Convert positive integer to string
*
*Purpose:
*       Convert positive integer to a string and store it in the output
*       buffer with no null terminator.  Update both the count and
*       buffer pointers.
*
*       Differs from _store_num in that the precision is not specified,
*       and no leading zeros are added.
*
*       *** For internal use with strftime() only ***
*
*       Created from xtoi.c
*
*Entry:
*       int num = pointer to integer value
*       char **out = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*
*Exit:
*       none
*
*Exceptions:
*       The buffer is filled until it is out of space.  There is no
*       way to tell beforehand (as in _store_num) if the buffer will
*       run out of space.
*
*******************************************************************************/

static void __cdecl _store_number (
        int num,
        char **out,
        size_t *count
        )
{
        char *p;                /* pointer to traverse string */
        char *firstdig;         /* pointer to first digit */
        char temp;              /* temp char */

        p = *out;

        /* put the digits in the buffer in reverse order */
        if (*count > 1)
        {
                do {
                        *p++ = (char) (num % 10 + '0');
                        (*count)--;
                } while ((num/=10) > 0 && *count > 1);
        }

        firstdig = *out;                /* firstdig points to first digit */
        *out = p;                       /* return pointer to next space */
        p--;                            /* p points to last digit */

        /* reverse the buffer */
        do {
                temp = *p;
                *p-- = *firstdig;
                *firstdig++ = temp;     /* swap *p and *firstdig */
        } while (firstdig < p);         /* repeat until halfway */
}


/***
*_store_winword() - Store date/time in WinWord format
*
*Purpose:
*       Format the date/time in the supplied WinWord format
*       and store it in the supplied buffer.
*
*       *** For internal use with strftime() only ***
*
*       The WinWord format is converted token by token to
*       strftime conversion specifiers.  _expandtime is then called to
*       do the work.  The WinWord format is expected to be a
*       character string (not wide-chars).
*
*Entry:
*       const char **format = address of pointer to WinWord format
*       const struct tm *tmptr = pointer to time/date structure
*       char **out = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*
*Exit:
*       none
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_winword (
        const char *format,
        const struct tm *tmptr,
        char **out,
        size_t *count
        )
{
        char specifier;
        const char *p;
        int repeat;

        while (*format && *count != 0)
        {
                specifier = 0;          /* indicate no match */
                _no_lead_zeros = 0;     /* default is print leading zeros */

                /* count the number of repetitions of this character */
                for (repeat=0, p=format; *p++ == *format; repeat++);
                /* leave p pointing to the beginning of the next token */
                p--;

                /* switch on ascii format character and determine specifier */
                switch (*format)
                {
                        case 'M':
                                switch (repeat)
                                {
                                        case 1: _no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'm'; break;
                                        case 3: specifier = 'b'; break;
                                        case 4: specifier = 'B'; break;
                                } break;
                        case 'd':
                                switch (repeat)
                                {
                                        case 1: _no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'd'; break;
                                        case 3: specifier = 'a'; break;
                                        case 4: specifier = 'A'; break;
                                } break;
                        case 'y':
                                switch (repeat)
                                {
                                        case 2: specifier = 'y'; break;
                                        case 4: specifier = 'Y'; break;
                                } break;
                        case 'h':
                                switch (repeat)
                                {
                                        case 1: _no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'I'; break;
                                } break;
                        case 'H':
                                switch (repeat)
                                {
                                        case 1: _no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'H'; break;
                                } break;
                        case 'm':
                                switch (repeat)
                                {
                                        case 1: _no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'M'; break;
                                } break;
                        case 's': /* for compatibility; not strictly WinWord */
                                switch (repeat)
                                {
                                        case 1: _no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'S'; break;
                                } break;
                        case 'A':
                        case 'a':
                                if (!_stricmp(format, "am/pm"))
                                        p = format + 5;
                                else if (!_stricmp(format, "a/p"))
                                        p = format + 3;
                                specifier = 'p';
                                break;
                        case '\'': /* literal string */
                                if (repeat & 1) /* odd number */
                                {
                                        format += repeat;
                                        while (*format && *count != 0)
                                        {
                                                if (*format == '\'')
                                                {
                                                        format++;
                                                        break;
                                                }
                                                if (isleadbyte((int)*format))
                                                {
                                                        *(*out)++ = *format++;
                                                        (*count)--;
                                                }
                                                *(*out)++ = *format++;
                                                (*count)--;
                                        }
                                }
                                else { /* even number */
                                        format += repeat;
                                }
                                continue;

                        default: /* non-control char, print it */
                                break;
                } /* switch */

                /* expand specifier, or copy literal if specifier not found */
                if (specifier)
                {
                        _expandtime(specifier, tmptr, out, count);
                        format = p; /* bump format up to the next token */
                } else {
                        if (isleadbyte((int)*format))
                        {
                                *(*out)++ = *format++;
                                (*count)--;
                        }
                        *(*out)++ = *format++;
                        (*count)--;
                }
        } /* while */
}


#else  /* _WIN32 */


#endif  /* #ifndef WIN32 */
