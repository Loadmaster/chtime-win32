//==============================================================================
// chtime.cpp
//	Change the modification timestamp of one or more filenames.
//
// Notes
//	This program is written for Microsoft Win32 only.
//
//	Compile with macro 'DEBUGS' defined to a nonzero value to enable
//	debugging output.
//
// Copyright ©2000-2003 by David R. Tribble, all rights reserved.
//------------------------------------------------------------------------------


// Identification

static char		ID[] =
    "@(#)drt/src/cmd/chtime.cpp $Revision: 1.5 $ $Date: 2010/03/12 18:39:14 $\n";

static char		COPYRIGHT[] =
    "@(#)Copyright ©2000-2010 by David R. Tribble, all rights reserved.\n";

#define ID_PROG		"chtime"
#define ID_VERS		"1.5"
#define ID_DATE		"2010-03-12"

#ifdef DEBUGS
 #undef  DEBUGS
 #define DEBUGS		1
#else
 #define DEBUGS		0
#endif


// System includes

#ifndef _WIN32
 #error Compile this under Win32 only
#endif

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define WIN32_LEAN_AND_MEAN	1
#include <windows.h>


// Local includes

// (None)


// Local constants

#define and		&&
#define or		||
#define not		!

#if 0
 #define STD		std
#else
 #define STD		/**/
#endif


//------------------------------------------------------------------------------
// class Program
//	Embodies the execution of this entire program.
//------------------------------------------------------------------------------

#define Program_VS	105			// Class version, 1.5

class Program
{
public: // Shared constants
    enum ExitCodes				// Program exit codes
    {
        RC_OKAY =	0,	// Success
        RC_READ =	1,	// Can't open/read a file
        RC_WRITE =	2,	// Can't open/write a file
        RC_TIME =	3,	// Can't get timestamp of a file
        RC_FIND =	4,	// Can't find wildcarded filename
        RC_CREATE =	5,	// Can't create a new file
        RC_USAGE =	127	// Improper command usage
    };

private: // Shared constants
    static const char *const
			s_usage[];		// Usage messages

private: // Variables
    bool		m_opt_verbose;		// Display verbose output
    bool		m_opt_create;		// Create nonexistent files
    bool		m_opt_localTime;	// Use local time, not UTC
    int			m_opt_year;		// Changed year number
    int			m_opt_mon;		// Changed month number
    int			m_opt_mday;		// Changed day of the month
    int			m_opt_hour;		// Changed hour number
    int			m_opt_min;		// Changed minute number
    int			m_opt_sec;		// Changed second number
    int			m_opt_msec;		// Changed millisecond number
    const char *	m_opt_fname;		// Model file name
    FILETIME		m_opt_fname_tm;		// Model file timestamp

public: // Functions
    /*void*/		~Program();		// Destructor
    /*void*/		Program();		// Default constructor

    int			main(int agrc, const char *const *argv);
						// Execute this program

private: // Static functions
    static void		usage();		// Print usage msg and punt
    static bool		parseDate(const char *date, SYSTEMTIME *st);
						// Parse a date specification

private: // Functions
    // Constructors and destructors not provided
    /*void*/		Program(const Program &o);
						// Copy constructor
    const Program &	operator =(const Program &o);
						// Assignment operator

    int			parseOpts(int argc, const char *const *argv);
						// Parse command line options
    int			changeFiles(const char *fname);
						// Change times of some files
    int			changeFile(const char *fname);
						// Change time of a filename
};


//------------------------------------------------------------------------------
// Program::Program()
//	Default constructor.
//------------------------------------------------------------------------------

/*void*/ Program::Program():
    m_opt_verbose(true),
    m_opt_create(false),
    m_opt_localTime(true),
    m_opt_year(-1),
    m_opt_mon(-1),
    m_opt_mday(-1),
    m_opt_hour(-1),
    m_opt_min(-1),
    m_opt_sec(-1),
    m_opt_msec(-1),
    m_opt_fname(NULL),
    m_opt_fname_tm()
{
#if Program_VS != 105
 #error Class Program has changed
#endif

    // Initialize
    ::GetSystemTimeAsFileTime(&m_opt_fname_tm);
}


//------------------------------------------------------------------------------
// Program::~Program()
//	Destructor.
//------------------------------------------------------------------------------

/*void*/ Program::~Program()
{
#if Program_VS != 105
 #error Class Program has changed
#endif

    // Nothing to do
}


//------------------------------------------------------------------------------
// Program::usage()
//	Display a command usage message, then punt.
//
// Returns
//	Does not return, but terminates the program by calling 'std::exit()'.
//------------------------------------------------------------------------------

/*static*/
void Program::usage()
{
    int		i;

    // Display a command usage message
    for (i = 0;  s_usage[i] != NULL;  i++)
        STD::fprintf(stdout, "%s\n", s_usage[i]);
    STD::fflush(stdout);

    // Punt
    STD::exit(RC_USAGE);
}

/*static*/
const char *const	Program::s_usage[] =
{
    "[" ID_PROG ", " ID_VERS " " ID_DATE "]",
    "",
    "Change the modification timestamps of one or more files.",
    "",
    "usage:  " ID_PROG " [-option...] file...",
    "",
    "Options:",
    "    -c              "
        "Create new files if they do not already exist.",
    "    -d DD           "
        "Change the day of the month to 'DD'.",
    "    -f FILE         "
        "Change the timestamps to the modification time of 'FILE'.",
    "    -m MM           "
        "Change the month to 'MM'.",
/*+++INCOMPLETE
    "    -R              "
        "Recursively change files within subdirectories.",
+++*/
    "    -s              "
        "Silent (not verbose) output.",
    "    -t TIME         "
        "Change the timestamps to 'TIME', which is of the format:",
    "                    "
        "    [CC]YY-MM-DD[.hh:mm[:ss[.uuu]]]",
    "    -u              "
        "Timestamps are specified as UTC, not local time.",
    "    -v              "
        "Verbose output (default).",
    "    -y [CC]YY       "
        "Change the year to 'CCYY'.",
    "",
    "If no date or '-f' option is specified, the current date and time will be "
        "used.",
    "",
    "The '-f' option cannot be specified if the '-t', '-y', '-m', or '-d' "
        "options",
    "are also specified.",
    "",
    "Filenames may contain wildcard characters ('?' and '*').",
    NULL
};


//------------------------------------------------------------------------------
// Program::parseDate()
//	Parse a date/time specification.
//
// Param	date
//	A date/time specification of the form:
//	    "[CC]YY-MM-DD[.hh:mm[:ss[.uuu]]]"
//
// Param	st
//	Pointer to a system time structure, which is filled in with the parsed
//	values of string 'date'.
//
// Returns
//	True if the date/time specification 'date' is correctly formed,
//	otherwise false.
//------------------------------------------------------------------------------

/*static*/
bool Program::parseDate(const char *date, SYSTEMTIME *st)
{
    const char *	s;
    char		buf[10+1];
    int			v;
    int			i;
    bool		done =	false;

    // Initialize
    STD::memset(st, 0, sizeof(*st));

    // Extract the '[CC]YY' (year) portion
    s = date;
    for (i = 0;  STD::isdigit(*s)  and  i < 8;  s++)
        buf[i++] = *s;
    if (*s == '\0'  or  i < 2  or  i > 4)
        return false;
    buf[i] = '\0';
    v = STD::atoi(buf);
    if (v < 100)
        v += 2000;
    if (v < 1900  or  v > 2100)
        return false;
    st->wYear = v;

    // Extract the 'MM' (month) portion
    s++;
    for (i = 0;  STD::isdigit(*s)  and  i < 8;  s++)
        buf[i++] = *s;
    if (*s == '\0'  or  i < 1  or  i > 2)
        return false;
    buf[i] = '\0';
    v = STD::atoi(buf);
    if (v < 1  or  v > 12)
        return false;
    st->wMonth = v;

    // Extract the 'DD' (day) portion
    s++;
    for (i = 0;  STD::isdigit(*s)  and  i < 8;  s++)
        buf[i++] = *s;
    if (i < 1  or  i > 2)
        return false;
    buf[i] = '\0';
    v = STD::atoi(buf);
    if (v < 1  or  v > 31)
        return false;
    st->wDay = v;

    if (*s == '\0')
        return true;

    // Extract the 'hh' (hour) portion
    s++;
    for (i = 0;  STD::isdigit(*s)  and  i < 8;  s++)
        buf[i++] = *s;
    if (*s == '\0'  or  i < 1  or  i > 2)
        return false;
    buf[i] = '\0';
    v = STD::atoi(buf);
    if (v < 0  or  v > 23)
        return false;
    st->wHour = v;

    // Extract the 'mm' (minute) portion
    s++;
    for (i = 0;  STD::isdigit(*s)  and  i < 8;  s++)
        buf[i++] = *s;
    if (i < 1  or  i > 2)
        return false;
    buf[i] = '\0';
    v = STD::atoi(buf);
    if (v < 0  or  v > 59)
        return false;
    st->wMinute = v;

    if (*s == '\0')
        return true;

    // Extract the 'ss' (second) portion
    s++;
    for (i = 0;  STD::isdigit(*s)  and  i < 8;  s++)
        buf[i++] = *s;
    if (i < 1  or  i > 2)
        return false;
    buf[i] = '\0';
    v = STD::atoi(buf);
    if (v < 0  or  v > 59)
        return false;
    st->wSecond = v;

    if (*s == '\0')
        return true;

    // Extract the 'uuu' (millisecond) portion
    s++;
    for (i = 0;  STD::isdigit(*s)  and  i < 8;  s++)
        buf[i++] = *s;
    if (i < 1  or  i > 3)
        return false;
    buf[i] = '\0';
    v = STD::atoi(buf);
    if (v < 0  or  v > 999)
        return false;
    st->wMilliseconds = v;

    if (*s != '\0')
        return false;

    // Success
    return true;
}


//------------------------------------------------------------------------------
// Program::parseOpts()
//	Parse the command lien options.
//
// Param	argc
//	The size of array 'argv'.
//
// Param	argv
//	The command line argument strings.
//
// Returns
//	The number of command line args parsed.
//------------------------------------------------------------------------------

int Program::parseOpts(int argc, const char *const *argv)
{
    int		optind;

    // Parse the command line options
    optind = 1;
    while (optind < argc  and  argv[optind][0] == '-')
    {
        const char *	opt;
        const char *	optarg;

        // Parse the next command line arg
        opt = &argv[optind][1];
        if (opt[0] == '\0')
            return optind;

        while (opt[0] != '\0')
        {
            int		optch;
            SYSTEMTIME	st;

            // Parse the next command line option
            optch = opt[0];
            if (optch == '\0')
                break;

            if (opt[1] != '\0')
                optarg = &opt[1];
            else
                optarg = &argv[optind+1][0];

            switch (optch)
            {
            case 'c':
                // Create files if they do not already exist
                m_opt_create = true;
                break;

            case 'd':
                // Change the day of the month of the timestamp
                if (m_opt_fname != NULL)
                    usage();
                m_opt_mday = STD::atoi(optarg);
                goto next_arg;

            case 'f':
                // Use the modification time from a specified filename
                if (m_opt_year != -1  or  m_opt_mon != -1  or  m_opt_mday != -1)
                    usage();
                m_opt_fname = optarg;
                goto next_arg;

            case 'm':
                // Change the month of the timestamp
                if (m_opt_fname != NULL)
                    usage();
                m_opt_mon = STD::atoi(optarg);
                goto next_arg;

            case 's':
                // Do not display verbose output
                m_opt_verbose = false;
                break;

            case 't':
                // Use a specific date/time
                if (m_opt_fname != NULL)
                    usage();
                if (not parseDate(optarg, &st))
                {
                    STD::fprintf(stderr, "Bad date specification: %s\n",
                        optarg);
                    STD::fflush(stderr);
                    usage();
                }

                m_opt_year = st.wYear;
                m_opt_mon =  st.wMonth;
                m_opt_mday = st.wDay;
                m_opt_hour = st.wHour;
                m_opt_min =  st.wMinute;
                m_opt_sec =  st.wSecond;
                m_opt_msec = st.wMilliseconds;
                goto next_arg;

            case 'u':
                // Use UTC, not local timezone
                m_opt_localTime = false;
                break;

            case 'v':
                // Display verbose output
                m_opt_verbose = true;
                break;

            case 'y':
                // Change the year of the timestamp
                if (m_opt_fname != NULL)
                    usage();
                m_opt_year = STD::atoi(optarg);
                if (m_opt_year < 100)
                    m_opt_year += 2000;
                goto next_arg;

            case 'h':
            case 'H':
            case '?':
                // Display command help
                usage();
                break;

            default:
                // Unknown command line option
                STD::fprintf(stderr, "Unknown option '-%c'\n", optch);
                STD::fflush(stderr);
                usage();
                break;
            }

        next_opt:
            // Advance to the next option char in this command line arg
            opt++;
        }

    next_arg:
        // Advance to the next command line arg
        if (optarg == &opt[1])
            optind++;
        else
            optind += 2;
    }

    // Done
    return optind;
}


//------------------------------------------------------------------------------
// Program::changeFile()
//	Change the timestamp of a filename.
//
// Param	fname
//	The name of a file to change.
//
// Returns
//	'Program::RC_OKAY' (zero) on success, or one of the 'Program::RC_XXX'
//	status codes on failure.
//------------------------------------------------------------------------------

int Program::changeFile(const char *fname)
{
    int		err;
    HANDLE	h;
    FILETIME	mtime;
    SYSTEMTIME	mt;

    // Open the (existing) file
    h = ::CreateFile(
            (LPCSTR) fname,
            (DWORD) GENERIC_WRITE,	// Write required to change a timestamp
            (DWORD) FILE_SHARE_READ,
            (LPSECURITY_ATTRIBUTES) NULL,
            (DWORD) OPEN_EXISTING,
            (DWORD) 0,
            (HANDLE) NULL);

    if (h == INVALID_HANDLE_VALUE)
    {
        STD::fprintf(stderr, "Can't write: %s\n", fname);
        STD::fflush(stderr);
        h = (HANDLE) NULL;
        err = RC_WRITE;
        goto fail;
    }

    // Retrieve the file's timestamp
    if (not ::GetFileTime(h, (FILETIME *) NULL, (FILETIME *) NULL, &mtime))
    {
        STD::fprintf(stderr, "Can't get timestamp for: %s\n", fname);
        STD::fflush(stderr);
        err = RC_TIME;
        goto fail;
    }

#if DEBUGS
    ::FileTimeToSystemTime(&mtime, &mt);
    STD::printf("$ stamp: %04d-%02d-%02d %02d:%02d:%02d.%03d Z\n",
        mt.wYear, mt.wMonth, mt.wDay,
        mt.wHour, mt.wMinute, mt.wSecond, mt.wMilliseconds);
#endif

    // Convert the timestamp into a broken-down form
    if (m_opt_localTime)
    {
        if (not ::FileTimeToLocalFileTime(&mtime, &mtime))
        {
            STD::fprintf(stderr, "Can't get local timestamp for: %s\n",
                fname);
            STD::fflush(stderr);
            err = RC_TIME;
            goto fail;
        }
    }

    if (not ::FileTimeToSystemTime(&mtime, &mt))
    {
        STD::fprintf(stderr, "Can't convert timestamp for: %s\n", fname);
        STD::fflush(stderr);
        err = RC_TIME;
        goto fail;
    }

#if DEBUGS
    STD::printf("$ stamp: %04d-%02d-%02d %02d:%02d:%02d.%03d LOC\n",
        mt.wYear, mt.wMonth, mt.wDay,
        mt.wHour, mt.wMinute, mt.wSecond, mt.wMilliseconds);
#endif

    // Set the file's timestamp
    if (m_opt_fname != NULL)
    {
        // Set the file's timestamp to the model file's modification time
        mtime = m_opt_fname_tm;
    }
    else
    {
        // Change the file's timestamp
        if (m_opt_year != -1)
            mt.wYear = m_opt_year;
        if (m_opt_mon != -1)
            mt.wMonth = m_opt_mon;
        if (m_opt_mday != -1)
            mt.wDay = m_opt_mday;
        if (m_opt_hour != -1)
            mt.wHour = m_opt_hour;
        if (m_opt_min != -1)
            mt.wMinute = m_opt_min;
        if (m_opt_sec != -1)
            mt.wSecond = m_opt_sec;
        if (m_opt_msec != -1)
            mt.wMilliseconds = m_opt_msec;

#if DEBUGS
        STD::printf("$ stamp: %04d-%02d-%02d %02d:%02d:%02d.%03d NEW\n",
            mt.wYear, mt.wMonth, mt.wDay,
            mt.wHour, mt.wMinute, mt.wSecond, mt.wMilliseconds);
#endif

        // Convert the broken-down timestamp into a filestamp
        if (not ::SystemTimeToFileTime(&mt, &mtime))
        {
            STD::fprintf(stderr, "Can't convert file timestamp for: %s\n",
                fname);
            STD::fflush(stderr);
            err = RC_TIME;
            goto fail;
        }

        if (m_opt_localTime)
        {
            if (not ::LocalFileTimeToFileTime(&mtime, &mtime))
            {
                STD::fprintf(stderr, "Can't convert local timestamp for: %s\n",
                    fname);
                STD::fflush(stderr);
                err = RC_TIME;
                goto fail;
            }
        }
    }

    // Update the file's modification timestamp
    if (not ::SetFileTime(h, (FILETIME *) NULL, (FILETIME *) NULL, &mtime))
    {
        STD::fprintf(stderr, "Can't modify timestamp for: %s\n", fname);
        STD::fflush(stderr);
        err = RC_TIME;
        goto fail;
    }

    // Display the change
    if (m_opt_verbose)
    {
        // Retrieve the file's new timestamp
        if (not ::GetFileTime(h, (FILETIME *) NULL, (FILETIME *) NULL, &mtime))
        {
            STD::fprintf(stderr, "Can't get new timestamp for: %s\n", fname);
            STD::fflush(stderr);
            err = RC_TIME;
            goto fail;
        }

        if (m_opt_localTime)
        {
            if (not ::FileTimeToLocalFileTime(&mtime, &mtime))
            {
                STD::fprintf(stderr, "Can't get local timestamp for: %s\n",
                    fname);
                STD::fflush(stderr);
                err = RC_TIME;
                goto fail;
            }
        }

        // Display the file's new timestamp
        ::FileTimeToSystemTime(&mtime, &mt);
        STD::fprintf(stdout, "%04d-%02d-%02d %02d:%02d:%02d.%03d  ",
            mt.wYear,
            mt.wMonth,
            mt.wDay,
            mt.wHour,
            mt.wMinute,
            mt.wSecond,
            mt.wMilliseconds);
        STD::fprintf(stdout, "%s\n", fname);
        STD::fflush(stdout);
    }

    // Done, clean up
    ::CloseHandle(h);
    h = (HANDLE) NULL;

    return RC_OKAY;

fail:
    // Failure, clean up
    if (h != (HANDLE) NULL)
        ::CloseHandle(h);
    h = (HANDLE) NULL;

    return err;
}


//------------------------------------------------------------------------------
// Program::changeFiles()
//	Change the timestamp of several files matching a wildcarded filename.
//
// Param	pat
//	Filename pattern matching the filenames to change.
//
// Returns
//	'Program::RC_OKAY' (zero) on success, or one of the 'Program::RC_XXX'
//	status codes on failure.
//------------------------------------------------------------------------------

int Program::changeFiles(const char *pat)
{
    int			err = RC_OKAY;
    HANDLE		fh;
    WIN32_FIND_DATA	fs;
    char *		fnamep;
    char		fpath[32*1024+1];

    // Set up the filename search
    fh = ::FindFirstFile((LPCSTR) pat, (WIN32_FIND_DATA *) &fs);

    if (fh == INVALID_HANDLE_VALUE)
    {
        if (m_opt_create)
        {
            const char *	fname;
            HANDLE		h;

            // Create a new file with the current date/time
            fname = pat;
            h = ::CreateFile(
                    (LPCSTR) fname,
                    (DWORD) GENERIC_WRITE,
                    (DWORD) FILE_SHARE_READ,
                    (LPSECURITY_ATTRIBUTES) NULL,
                    (DWORD) CREATE_NEW,
                    (DWORD) 0,
                    (HANDLE) NULL);

            if (h == INVALID_HANDLE_VALUE)
            {
                // Can't create a new file (possibly a wildcard name)
                STD::fprintf(stderr, "Can't create: %s\n", fname);
                STD::fflush(stderr);
                err = RC_CREATE;
            }
            else
            {
                ::CloseHandle(h);

                // Modify the timestamp of the new file
                err = changeFile(fname);
            }

            return err;
        }
        else
        {
            // Can't find any matching filenames
            STD::fprintf(stderr, "Can't find: %s\n", pat);
            STD::fflush(stderr);
            fh = (HANDLE) NULL;
            err = RC_FIND;
            goto fail;
        }
    }

    // Initialize the full filename path
    {
        // Copy and normalize the filename pattern
        for (int j = 0;  pat[j] != '\0';  j++)
        {
            fpath[j] = pat[j];
            if (fpath[j] == '/')
                fpath[j] = '\\';
        }

        // Truncate the leading directory path prefix
        fnamep = STD::strrchr(fpath, '\\');
        if (fnamep == NULL)
            fnamep = fpath;
        else
            fnamep++;
        *fnamep = '\0';
    }

    // Search for one or more matching filenames
    for (;;)
    {
        const char *	fname;
        int		rc;

        // Change the timestamp of the next matching filename
        fname = (char *) fs.cFileName;
        STD::strcpy(fnamep, fname);
        rc = changeFile(fpath);

        if (err == RC_OKAY)
            err = rc;

        // Search for the next matching filename
        if (not ::FindNextFile(fh, (WIN32_FIND_DATA *) &fs))
            break;
    }

    // Clean up
    ::FindClose(fh);
    fh = (HANDLE) NULL;

    return err;

fail:
    // Failure, clean up
    if (fh != (HANDLE) NULL)
        ::FindClose(fh);
    fh = (HANDLE) NULL;

    return err;
}


//------------------------------------------------------------------------------
// Program::main()
//
// Param	argc
//	The size of array 'argv'.
//
// Param	argv
//	The command line argument strings.
//
// Returns
//	'Program::RC_OKAY' (zero) on success, or one of the 'Program::RC_XXX'
//	status codes on failure.
//------------------------------------------------------------------------------

int Program::main(int argc, const char *const *argv)
{
    HANDLE	h =	(HANDLE) NULL;
    int		err =	RC_OKAY;
    int		i;

    // Parse command line options
    i = parseOpts(argc, argv);
    argc -= i;
    argv += i;

    // Check args
    if (argc < 1)
        usage();

    // Process options
    if (m_opt_fname != NULL)
    {
        // Open the existing model file
        h = ::CreateFile(
                (LPCSTR) m_opt_fname,
                (DWORD) GENERIC_READ,
                (DWORD) FILE_SHARE_READ,
                (LPSECURITY_ATTRIBUTES) NULL,
                (DWORD) OPEN_EXISTING,
                (DWORD) 0,
                (HANDLE) NULL);

        if (h == INVALID_HANDLE_VALUE)
        {
            STD::fprintf(stderr, "Can't read: %s\n", m_opt_fname);
            STD::fflush(stderr);
            h = (HANDLE) NULL;
            err = RC_READ;
            goto done;
        }

        // Retrieve the model file's timestamp
        if (not ::GetFileTime(h, (FILETIME *) NULL, (FILETIME *) NULL,
                &m_opt_fname_tm))
        {
            STD::fprintf(stderr, "Can't get timestamp for: %s\n", m_opt_fname);
            STD::fflush(stderr);
            err = RC_TIME;
            goto done;
        }

        // Clean up
        ::CloseHandle(h);
        h = (HANDLE) NULL;
    }
    else if (m_opt_year == -1  and  m_opt_mon == -1  and  m_opt_mday == -1)
    {
        // Use the current date/time
        m_opt_fname = "";
    }

    // Process filename args
    for (i = 0;  i < argc;  i++)
    {
        int	rc;

        // Change timestamps of several wildcarded filenames
        rc = changeFiles(argv[i]);

        if (err == RC_OKAY)
            err = rc;
    }

done:
    // Done, clean up
    if (h != (HANDLE) NULL)
        ::CloseHandle(h);
    h = (HANDLE) NULL;

    // Done
    STD::exit(err);
    return err;
}


//------------------------------------------------------------------------------
// ::main()
//
// Param	argc
//	The size of array 'argv'.
//
// Param	argv
//	The command line argument strings.
//
// Returns
//	'Program::RC_OKAY' (zero) on success, or one of the 'Program::RC_XXX'
//	status codes on failure.
//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    Program	pgm;

    return (pgm.main(argc, (const char *const *) argv));
}

// End chtime.cpp
