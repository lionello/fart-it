//  Copyright (C) 1998-2002 Lionello Lunesu.  All rights reserved.
//  
//  FART is distributed with NO WARRANTY OF ANY KIND.  No author or
//  distributor accepts any responsibility for the consequences of using it,
//  or for whether it serves any particular purpose or works at all, unless
//  he or she says so in writing.  Refer to the GNU General Public
//  License (the "License") for full details.

//  FART <wc>					Show files that comply with <wc> + final count (find)

//  FART <wc> <s>				Find files <wc>, echo lines containing <s> [+ count] (grep)
//  FART -! <wc> <s>			Find files <wc>, echo lines NOT containing <s> [+ count]
//  FART - <s>					Echo lines from stdin containing <s> + final count
//  FART -! - <s>				Echo lines from stdin NOT containing <s> + final count

//  FART <wc> <s> <r>			Find files <wc> with <s>, replace with <r>, show filenames + count
//  FART -p <wc> <s> <r>		Find files <wc> with <s>, show filenames, print lines with <r> + count
//  FART <wc> <s> "r"			Find files <wc>, remove lines with <s>, show filenames + count
//  FART - <s> <r>				Echo lines from stdin containing <s>, but show <r> + final count

// TODO:
// * don't touch files if nothing changed
// * don't use temp file, unless needed
// * prevent processing a file twice when fart'ing filenames
// * rename folders

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <fcntl.h>
#include <conio.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#endif
#ifdef __unix__
#include "fart_linux.h"
#endif

///////////////////////////////////////////////////////////////////////////////

#define VERSION				"v1.96"
#define TEMPFILE			"_fart.~"

#define WILDCARD_SEPARATOR	','
#define WILDCARD_ALL		"*"

#ifdef _WIN32

# define DIR_SEPARATOR		'\\'
# define DRIVE_SEPARATOR	':'
# define DIR_CURRENT		""
# define DIR_PARENT			"..\\"
/*
typedef _finddata_t DIRENT;
# define DIRENT_NAME(d)		(d)->name
# define DIRENT_ISDIR(d)	((d)->attrib & _A_SUBDIR)

typedef long DIR;
# define OPENDIR(d,data)	_findfirst(d,data)
# define READDIR(d)			_findnext(d);
# define CLOSEDIR(d)		_findclose(d);
*/
#else // _WIN32

extern "C" int wildmat (const char *text, const char *p);
# define DIR_SEPARATOR		'/'
# define DIR_CURRENT		""
# define DIR_PARENT			"../"
/*
typedef dirent DIRENT;
# define DIRENT_NAME(d)		(d)->d_name
# define DIRENT_ISDIR(d)	((d)->d_type==DT_DIR)

# define OPENDIR(a,b,c)		opendir(),
*/

#endif // !_WIN32

static const char __dir_separator[] = {DIR_SEPARATOR,'\0'};

// Output strings (eventually customizable)
static char __linenumber[16] = "[%4i]";				// [   2]
static char __filename[16] = "%s\n";				// fart.cpp
static char __filename_count[16] = "%s [%i]\n";		// fart.cpp [2]
static char __filename_text[16] = "%s :\n";			// fart.cpp :
static char __filename_rename[16] = "%s => %s\n";	// fart.CPP => fart.cpp

bool	_Numbers = false;
bool	_Backup = false;
bool	_Preview = false;
bool	_Quiet = false;
bool	_Help = false;
bool	_IgnoreCase = false;
bool	_SubDir = false;
bool	_AdaptCase = false;
bool	_WholeWord = false;
bool	_CVS = false;
bool	_Verbose = false;
bool	_Invert = false;
bool	_Count = false; 
bool	_Names = false; 
bool	_Binary = false;
bool	_CStyle = false;

struct 
{
	bool*	state;
	char	option;
	char*	option_long;
	char*	description;
} arguments[] = {
	// general options
	{ &_Help, 'h', "help", "Show this help message (ignores other options)" },
	{ &_Quiet, 'q', "quiet", "Suppress output to stdio / stderr" },
	{ &_Verbose, 'V', "verbose", "Show more information" },
//	{ &_Options, ' ', "", "No more options after this" },			// --
	// find options
	{ &_SubDir, 'r', "recursive", "Process sub-folders recursively" },
	{ &_Count, 'c', "count", "Only show filenames, match counts and totals" },
	// grep options
//	{ &_Regex, 'g', "regex", "Interpret find_string as a basic regular expression" },
	{ &_IgnoreCase, 'i', "ignore-case", "Case insensitive text comparison" },
	{ &_Invert, 'v', "invert", "Print lines NOT containing the find string" },
	{ &_Numbers, 'n', "line-number", "Print line number before each line (1-based)" },
	{ &_WholeWord, 'w', "word", "Match whole word (uses C syntax, like grep)" },
	{ &_Names, 'f', "filename", "Find (and replace) filename instead of contents" },
	{ &_Binary, 'B', "binary", "Also search (and replace) in binary files (CAUTION)" },
	{ &_CStyle, 'C', "c-style", "Allow C-style extended characters (\\xFF\\0\\t\\n\\r\\\\ etc.)" },
	{ &_CVS, ' ', "cvs", "Skip cvs dirs; execute \"cvs edit\" before changing r/o files" },
	// fart specific options
//	{ &_VSS, ' ', 'vss", "Do SourceSafe check-out before changing r/o files" },
	{ &_AdaptCase, 'a', "adapt", "Adapt the case of replace_string to found string" },
	{ &_Backup, 'b', "backup", "Make a backup of each changed file" },
	{ &_Preview, 'p', "preview", "Do not change the files but print the changes" },
	// end
	{ NULL, 0, NULL, NULL } };

int		TotalFileCount;					// Number of files
int		TotalFindCount;					// Number of total occurences

#define MAXSTRING 1024

bool	HasWildCard = false;
char	WildCard[MAXSTRING];

bool	_DoubleCheck = false;
int		FindLength = 0;
char	FindString[MAXSTRING];
int		ReplaceLength = 0;
char	ReplaceString[MAXSTRING];

char	ReplaceStringLwr[MAXSTRING], ReplaceStringUpr[MAXSTRING];

char	fart_buf[MAXSTRING];

// Macro's for output to stderr, first flush stdout and later stderr
#define ERRPRINTF( s ) fflush(stdout),fprintf( stderr, s ),fflush(stderr)

#define ERRPRINTF1( s, a ) fflush(stdout),fprintf( stderr, s, a ),fflush(stderr)

#define ERRPRINTF2( s, a, b ) fflush(stdout),fprintf( stderr, s, a, b ),fflush(stderr)

///////////////////////////////////////////////////////////////////////////////
// Callback function for 'for_all_files' and 'for_all_files_recursive'

typedef int (*file_func_t)( const char* dir, const char* file );

///////////////////////////////////////////////////////////////////////////////

inline bool _iswordchar( char c )
{
//	return isalnum(c) || c=='_';
	return c=='_' || (c>='0'&&c<='9') || (c>='a'&&c<='z') || (c>='A'&&c<='Z');
}

///////////////////////////////////////////////////////////////////////////////
// Expands c-style character constants in the input string; returns new size
// (strlen won't work, since the string may contain premature \0's)

int cstyle( char *buffer )
{
	int len =0;
	char *cur = buffer;
	while (*cur)
	{
		if (*cur == '\\')
		{
			cur++;
			switch (*cur)
			{
			case 0: *buffer = '\0'; return len;
			case 'n': *buffer++ = '\n'; break;
			case 't': *buffer++ = '\t'; break;
			case 'v': *buffer++ = '\v'; break;
			case 'b': *buffer++ = '\b'; break;
			case 'r': *buffer++ = '\r'; break;
			case 'f': *buffer++ = '\f'; break;
			case 'a': *buffer++ = '\a'; break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			{
				int x, n=0;
				sscanf(cur,"%3o%n",&x,&n);
				cur += n-1;
				*buffer++ = (char)x;
				break;
			}
			case 'x':								// hexadecimal
			{
				int x, n=0;
				sscanf(cur+1,"%2x%n",&x,&n);
				if (n>0)
				{
					cur += n;
					*buffer++ = (char)x;
					break;
				}
				// seep through
			}
			default:  
				ERRPRINTF1( "Warning: unrecognized character escape sequence: \\%c\n", *cur );
			case '\\':
			case '\?':
			case '\'':
			case '\"':
				*buffer++ = *cur;
			}
			cur++;
		}
		else
			*buffer++ = *cur++;
		len++;
	}
	*buffer = '\0';
	return len;
}

///////////////////////////////////////////////////////////////////////////////
// Efficient strdup that concatenates 2 strings.

char* strdup2( const char* s1, const char* s2 )
{
	int l1 = strlen(s1);
	int l2 = strlen(s2) + 1;
	char *str = (char*)malloc(l1+l2);
	memcpy( str, s1, l1 );
	memcpy( str+l1, s2, l2 );
	return str;
}

///////////////////////////////////////////////////////////////////////////////
// Find memory block inside memory block

char* memmem( char* m1, size_t len1, const char *m2, size_t len2 )
{
	if (len1<len2)
		return NULL;
	// Check for valid arguments (same behaviour as strstr)
	if (!m2 || !len2)
		return m1;
	// Do a regular compare, if the buffers are of the same size
//	if (len1==len2)
//		return memcmp(m1,m2,len1)?NULL:m1;

	// TODO: could be improved somewhat (a.o. inlining memcmp)
	for (size_t t=0;t<=len1-len2;t++)
		if (m1[t]==m2[0])
		{
			if (memcmp( &m1[t], m2, len2 )==0)
				return &m1[t];
		}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Prepares a line for text comparison

const char* get_compare_buf( const char *in )
{
static char compare_buf[MAXSTRING];

	if (_IgnoreCase)
	{
		// Copy the text into an extra buffer (used just for comparison)
		strcpy( compare_buf, in );
		// Compare lowercase (assume FindString already is lowercase)
		strlwr( compare_buf );
		return compare_buf;
	}
	return in;
}

///////////////////////////////////////////////////////////////////////////////

char* memlwr( char *ptr, size_t size )
{
	char *cur = ptr;
	while (size)
	{
		strlwr(cur);
		char *z = (char*)memchr(cur,'\0',size);
		if (!z)
			break;
		size -= z+1-cur;
		cur = z+1;
	}
	return ptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void DumpHelp()
{
	// Print banner
	printf( "\nFind And Replace Text  %-30s by Lionello Lunesu\n\n",VERSION);

	printf("Usage: FART [options] [--] <wildcard>[%c...] [find_string] [replace_string]\n", WILDCARD_SEPARATOR );
	printf("\nOptions:\n");
	for (int t=0;arguments[t].state;t++)
		printf(" %c%c --%-14s%s\n", 
			arguments[t].option==' '?' ':'-', arguments[t].option, 
			arguments[t].option_long, arguments[t].description );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Analyzes the case of the 'in' string
//  -1=all lower case, 0=mixed, 1=all upper case

int analyze_case( const char* in, int inl )
{
	int UC=0, LC=0;

	for (int t=0;t<inl;t++)
	{
		char uc = toupper(in[t]);
		char lc = tolower(in[t]);

		if (uc==lc)			// char is not alphabetic
			continue;

		if (uc==in[t])		// char is uppercase
			UC++;
		else				// char is lowercase
			LC++;
	}

	if (UC==0 && LC==0)						// no alphabetic chars
		return 0;

	// If all the chars are either upper- or lowercase
	// we can get rid of the 'unknown' cases

	if (UC==0)
		return -1;							// all lowercase

	if (LC==0)
		return 1;							// all uppercase

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Method that prints a string with CR/LF temporarily cut off

void puts_nocrlf( char *_buf )
{
	int nl = strlen(_buf);

	while (nl>0 && (_buf[nl-1]=='\r' || _buf[nl-1]=='\n')) nl--;

	char o = _buf[nl];
	_buf[nl] = '\0';				// chop off the CR/LF
	puts(_buf);						// to stdout
	_buf[nl] = o;					// restore
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Returns the number of times the find string occurs in the input string

int findtext_line_count( const char *_line )
{
	const char *line = get_compare_buf(_line);

	int count = 0;
	const char *cur = line;
	const char *t;
	while ((t = strstr( cur, FindString )))
	{
		cur = t + FindLength;
		if (_WholeWord)
		{
			if (t>line && _iswordchar(t[-1]))
				continue;
			if (_iswordchar(t[FindLength]))
				continue;
		}
		count++;
	}
	return count;
}

///////////////////////////////////////////////////////////////////////////////
// Returns a pointer to the first occurence of the find string

const char* findtext_line( const char* _line )
{
	const char *line = get_compare_buf(_line);

	// Find the string in this line (FindString is lower case if _IgnoreCase)
	const char *cur = line;
	const char *t;
	while ((t = strstr( cur, FindString )))
	{
		if (_WholeWord)
		{
			cur = t + FindLength;
			if (t>line && _iswordchar(t[-1]))
				continue;
			if (_iswordchar(t[FindLength]))
				continue;
		}
		break;
	}
	return t;
}


///////////////////////////////////////////////////////////////////////////////
// Process input file while data available (filename is optional)

int _findtext( FILE* f, const char *filename )
{
	int	this_find_count=0;					// number of occurences in this file
	bool first = true;						// first occurence in this file?
	int ln=0;								// line number

	while (!feof(f))
	{
		ln++;
		if (!fgets( fart_buf, MAXSTRING, f ))
			break;

//		char *t = findtext_line( fart_buf );
		// no need to know the exact occurence, just count
		int t = findtext_line_count( fart_buf );

		if (_Invert)
		{
			if (t)
				continue;
			// count lines NOT containing the find_string
			t = 1;
		}
		else
		{
			if (!t)
				continue;
		}

		this_find_count += t;
			
		if (first)
		{
			// This is the first occurence in this file
			first = false;

			// If Q and C then we don't need the exact count, stop
			if (_Quiet && _Count)
				break;						// No need to continue

			if (!_Count && !_Quiet)
			{
				// We're dumping the lines after this
				if (filename)
					printf( __filename_text, filename );
			}
		}

		if (_Count)
			continue;

		if (_Numbers)
			printf( __linenumber, ln );

		puts_nocrlf( fart_buf );
	}
	return this_find_count;
}

///////////////////////////////////////////////////////////////////////////////

const char* pre_fart( const char* test )
{
	// TODO: make sure test[-1] is always valid and we can use this:
/*	if (_WholeWord)
	{
		if (t>compare_buf && _iswordchar(t[-1]))
			continue;
		if (_iswordchar(t[FindLength]))
			continue;
	}*/

	const char* replacement = ReplaceString;
	// find the correct (case-adapted) replace_string
	if (_AdaptCase)
	{
		int i = analyze_case(test,FindLength);
		if (i>0)
			replacement = ReplaceStringUpr;
		else
		if (i<0)
			replacement = ReplaceStringLwr;
	}

	// double-check to see whether anything will really changed
	if (_DoubleCheck)
		if (memcmp(test,replacement,FindLength)==0)
			return NULL;

	return replacement;
}

///////////////////////////////////////////////////////////////////////////////
// Find and replace text in one line. Returns NULL if nothing has changed

const char* fart_line( const char *_line, char *farted )
{
	const char *compare_buf = get_compare_buf(_line);

	farted[0]='\0';

	bool replaced = false;
	char *output = farted;
	size_t cur = 0, offset;
	const char *t;
	// Find the string in this line (FindString is lower case if _IgnoreCase)
	for (;(t = strstr( compare_buf+cur, FindString ));cur=offset+FindLength)
	{
		offset = t - compare_buf;
		if (_WholeWord)
		{
			if (t>compare_buf && _iswordchar(t[-1]))
				continue;
			if (_iswordchar(t[FindLength]))
				continue;
		}

		const char* replacement = pre_fart(_line+offset);
		if (!replacement)
			continue;

		// string was found at t
		replaced = true;
		// copy characters up-to t
		while (cur<offset)
			*output++ = _line[cur++];
		// append the replace string to the output
		for (int i=0;i<ReplaceLength;i++)
			*output++ = replacement[i];
		*output = '\0';
		// continue right after find string
	}
	if (!replaced)
		return NULL;
	// append the last part
	strcpy(output,_line+cur);
	return farted;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Returns 'true' if 'dir' is a directory used by CVS

bool is_cvs_path( const char* dir )
{
#ifdef _WIN32
	return strcmp(dir,"CVS")==0;			// stricmp?
#else
	return strcmp(dir,"cvs")==0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Returns 'true' if 'wc' is a file wildcard (containing * or ?)

bool is_wildcard( const char* wc )
{
	return strchr(wc,'*') || strchr(wc,'?');
}

///////////////////////////////////////////////////////////////////////////////
// Returns 'true' if the file seems binary, 'false' otherwise (text)

bool is_binary( FILE *f )
{
static char fout[256] = {0};

	if (!fout[0])
	{
		memset( fout, 1, 256 );					// not allowed
		memset( fout, 0, 128 );					// allowed <128
		memset( fout, 4, 32 );					// more expensive <32
		fout[0x9]=fout[0xA]=fout[0xD]=0;		// allowed: TAB CR LF
	}

	// check for binary file
	size_t tot = fread( fart_buf, 1, MAXSTRING, f );
	size_t noascii=0;
	for (size_t b=0;b<tot;b++)
	{
		unsigned char t = fart_buf[b];
		noascii += (size_t)fout[ t ];
	}

	// restore file pointer
	fseek(f,0,SEEK_SET);
//	freopen(in,"rb",f);

//	ERRPRINTF2("[%i/%i]",noascii,tot);
	return noascii*20>=tot;						// 5%
}

///////////////////////////////////////////////////////////////////////////////
// Find text in file
//  !DoKeepLine			Only show files NOT containing find_string
//  DoRemLine			Hide lines containing find_string

bool findtext_file( const char* in )
{
	// Open file for reading
	FILE *f = fopen(in,"rb");
	if (!f)
	{
		ERRPRINTF1( "Error: unable to open file %s\n", in );
		return false;
	}

	// check binary/text
	if (!_Binary && is_binary(f))
	{
		if (_Verbose)
			ERRPRINTF1( "FART: skipping binary file: %s\n", in );
		fclose(f);
		return false;
	}

	int file_find_count = _findtext(f,in);
	if (file_find_count) TotalFileCount++;
	TotalFindCount += file_find_count;

	// Close file handle
	fclose(f);

	if (_Count && file_find_count)
	{
		// Show filenames [+ count]
		if (_Quiet)
			printf( __filename, in );
		else
			printf( __filename_count, in, file_find_count );
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Find and replace text in file. Returns the number of replacements

int _fart( FILE *f1, FILE *f2, const char* in )
{
	int	this_find_count=0;					// Number of occurences in this file
	bool first = true;
	bool replace = false;
	int ln=0;

	// Process input file while data available
	while (!feof(f1))
	{
		ln++;
		if (!fgets( fart_buf, MAXSTRING, f1 ))
			break;

		const char* b = get_compare_buf(fart_buf);

		bool first_line=true;
		const char* bp = b;
		while (1)
		{
			char *t = strstr(bp,FindString);

			// Check for word boundary
			if (t && _WholeWord)
			{
				// FIXME: does not search for another occurence
				if (t>bp && _iswordchar(t[-1]))
					t = NULL;
				else
				if (_iswordchar(t[FindLength]))
					t = NULL;
			}

			if (!t)
			{
				// find_string not found
				fputs( fart_buf+(bp-b), f2 );
				break;
			}
			else
			{
				this_find_count++;
				TotalFindCount++;
			}

			// Adapt the replace_string to the actually found string
			const char *replacement = pre_fart( fart_buf+(t-b) );
			if (!replacement)
			{
				fputs( fart_buf+(bp-b), f2 );
				break;
			}

			// First occurence in this file?
			if (first)
			{
				TotalFileCount++;
				first = false;
				// Print the filename
				if (!_Count && !_Quiet && in)
					printf( __filename, in );
			}

			// Remove lines containing the find_string
/*			if (DoRemLine)
			{
				replace = true;
				break;
			}
*/
			// Don't replace find_string if DoKeepLine is NOT set
/*			if (!DoKeepLine)
			{
				if (!t) 
				{
					// Skip this line (doesn't contain find_string)
					replace = true;
				}
				else
					fputs( fart_buf+(bp-b), f2 );
				break;
			}
*/
			// First occurence in this line?
			if (first_line)
			{
				if (_Numbers)
					printf(__linenumber, ln );
//				if (!_Quiet)
//					puts_nocrlf(fart_buf);
				first_line = false;
			}

			// Put a '\0' just before the find_string
//			*(fart_buf+(t-b)) = '\0';
			// Write the text before the find_string
			fwrite( fart_buf+(bp-b), 1, t-bp, f2 );
			// Write the replace_string instead of the find_string
			fwrite( replacement, 1, ReplaceLength, f2 );
			// Put the buffer-pointer right after the find_string
			bp = t + FindLength;

			// Something got replaced
			replace = true;
		}
	}
	return replace?this_find_count:0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Find and replace text in file (uses temporary file)

bool fart( const char* in )
{
	// Open original file for reading
	FILE *f1 = fopen(in,"rb");
	if (!f1)
	{
		ERRPRINTF1( "Error: unable to open file %s\n", in );
		return false;
	}

	// check binary/text
	if (!_Binary && is_binary(f1))
	{
		if (_Verbose)
			ERRPRINTF1( "FART: skipping binary file: %s\n", in );
		fclose(f1);
		return false;
	}

	// Open temporary file for writing
	FILE *f2 = fopen(TEMPFILE,"wb");
	if (!f2)
	{
		ERRPRINTF( "Error: unable to create temporary file\n" );
		fclose(f1);
		return false;
	}

	int this_find_count = _fart(f1,f2,in);

	// Close file handles
	fclose(f2);
	fclose(f1);

	if (this_find_count && _Count && !_Quiet)
	{
		// Show filenames + count
		printf( __filename_count, in, this_find_count );
	}

	if (this_find_count && !_Preview)
	{
		if (_CVS)
		{
			// TODO: check for read-only first
			if (_Verbose)
				ERRPRINTF1( "FART: cvs edit %s\n", in );
//			_spawnlp( _P_WAIT, "cvs", "cvs", "edit", in, NULL );
		}

		if (_Backup)
		{
			// Append ".bak" to filename
			char *backup = strdup2(in,".bak");
			// Remove old backup. Rename original file to backup-filename
			if (remove( backup )!=0)
				ERRPRINTF1( "Error: could not remove %s\n", backup );
			else
			if (rename( in, backup )!=0)
				ERRPRINTF1( "Error: could not backup %s\n", in );
			free(backup);
		}
		else
		{
			// Remove original file
			if (remove( in )!=0)
				ERRPRINTF1( "Error: file %s is read only\n", in );
		}
		// Rename temporary file to original filename
		//  (will fail if we could not rename/remove the original file)
		rename( TEMPFILE, in );
	}

	// Remove temporary file
	// (either nothing has changed, or we failed to rename/remove the original file)
	remove( TEMPFILE );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Search for files matching a wildcard
// FIXME: when fart'ing filenames, we might process a file twice!

int for_all_files( const char *dir, const char* wc, file_func_t _ff )
{
	_finddata_t fd;
	long fh;
	int count = 0;

	if (_Verbose)
		ERRPRINTF2( "FART: processing %s,%s\n",dir,wc);

	// Make full path wildcard
	char *_path = strdup2(dir,wc);
	fh = _findfirst( _path, &fd );
//	if (_Verbose && fh==-1)
//		ERRPRINTF1("FART: failed to open %s\n",_path);
	free(_path);

	for (int lf=(int)fh;lf!=-1;lf=_findnext(fh,&fd))
	{
		// Do files now; process folders later
		if (fd.attrib & _A_SUBDIR)
			continue;
#ifndef _WIN32
		if (strcmp(fd.name,wc) && !wildmat(fd.name,wc))
			continue;
#endif

		// Call callback function
//		if (_Verbose)
//			ERRPRINTF2( "FART: found %s,%s\n", dir, fd.name );
		count += _ff( dir, fd.name );
	}
	_findclose(fh);
	return count;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Recurse through directory and search for files matching a wildcard

int for_all_files_recursive( const char *dir, const char* wc, file_func_t _ff )
{
	// First, process the current directory
	int count = for_all_files(dir,wc,_ff);

	// Now we recurse into folders

	char _path[_MAX_PATH];				// TODO: save some stack space
	// Find all dirs (*.*)
	strcpy( _path, dir );
	strcat( _path, WILDCARD_ALL );

	_finddata_t fd;
	long fh = _findfirst( _path, &fd );
	for (int lf=(int)fh;lf!=-1;lf=_findnext(fh,&fd))
	{
		if (fd.attrib & _A_SUBDIR)
		{
			// Skip "."
			if (!strcmp(fd.name,"."))
				continue;
			// Skip ".."
			if (!strcmp(fd.name,".."))
				continue;
#ifndef _WIN32
			if (strcmp(fd.name,wc) && !wildmat(fd.name,wc))
				continue;
#endif
			// Don't recurse into cvs directories
			if (_CVS && is_cvs_path(fd.name))
			{
				if (_Verbose)
					ERRPRINTF2( "FART: skipping cvs dir %s%s\n",dir, fd.name );
				continue;
			}

			// No chdir; this would mess up the command line

			// Append subdir to current dir
			strcpy(_path,dir);
			strcat(_path,fd.name);

			strcat(_path,__dir_separator);

			count += for_all_files_recursive(_path,wc,_ff);
		}
	}
	_findclose(fh);
	return count;
}

///////////////////////////////////////////////////////////////////////////////

int print_files( const char* dir, const char* file )
{
	char *_path = strdup2(dir,file);

	// FIXME?: does not check whether file actually exists
	puts(_path);
	free(_path);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

int for_all_files_smart( const char* dir, const char* file, file_func_t _ff )
{
	if (!is_wildcard(file))
		return _ff(dir,file);

	if (_SubDir)
		return for_all_files_recursive( dir, file, _ff );
	else
		return for_all_files( dir, file, _ff );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int for_all_wildcards( char *wildcard, file_func_t _ff )
{
	char file[_MAX_PATH];
	int count = 0;
	while (1)
	{
		char *wc_sep = strchr(wildcard,WILDCARD_SEPARATOR);
		if (wc_sep)
			*wc_sep = '\0';

		char *dir_sep = strrchr(wildcard,DIR_SEPARATOR);
#ifdef DRIVE_SEPARATOR
		if (!dir_sep)
			dir_sep = strchr(wildcard,DRIVE_SEPARATOR);
#endif
		if (dir_sep)
		{
			dir_sep++;						// point after "dir/"
			if (*dir_sep)
			{
				strcpy( file, dir_sep );
				*dir_sep = '\0';
				count += for_all_files_smart( wildcard, file, _ff );
			}
			else
			{
				// No wildcard, assume ALL
				count += for_all_files_smart( wildcard, WILDCARD_ALL, _ff );
			}
		}
		else
		if (strcmp(wildcard,".")==0)
			count += for_all_files_smart( DIR_CURRENT, WILDCARD_ALL, _ff );
		else
		if (strcmp(wildcard,"..")==0)
			count += for_all_files_smart( DIR_PARENT, WILDCARD_ALL, _ff );
		else
			count += for_all_files_smart( DIR_CURRENT, wildcard, _ff );

		// No separator found? Finished.
		if (!wc_sep)
			break;

		*wc_sep = WILDCARD_SEPARATOR;		// restore
		wildcard = wc_sep + 1;				// next piece
	}
	return count;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void options_short( const char *options )
{
	while (options[0])
	{
//		char opt = _tolower(argv[t][1]);
		int tt;
		for (tt=0;arguments[tt].state;tt++)
			if (arguments[tt].option == options[0])
			{
				*arguments[tt].state = true;
				if (_Verbose)
					ERRPRINTF1( "FART: --%s\n", arguments[tt].option_long );
				break;
			}
		// Did we process the option?
		if (!arguments[tt].state)
		{
			if (options[0]!='?')				// don't show error for '?' 
				ERRPRINTF1( "Error: invalid option -%c\n", options[0] );
			_Help = true;
		}
		// Next option
		options++;
	}
}

///////////////////////////////////////////////////////////////////////////////

void options_long( const char *option )
{
	for (int tt=0;arguments[tt].state;tt++)
		if (strcmp(arguments[tt].option_long,option)==0)
		{
			*arguments[tt].state = true;
			if (_Verbose)
				ERRPRINTF1( "FART: --%s\n", arguments[tt].option_long );
			return;
		}
	ERRPRINTF1( "Error: invalid option --%s\n", option );
	_Help = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void parse_options( int argc, char* argv[] )
{
	bool do_options = true;
	for (int t=1;t<argc;t++)
	{
#ifdef _WIN32
		// Parse DOS style options
		if (do_options && argv[t][0]=='/')
		{
			options_short(argv[t]+1);
			continue;
		}
#endif

		// Parse options; "-" is NOT an option but file/text
		if (do_options && argv[t][0]=='-' && argv[t][1])
		{
			if (argv[t][1]=='-')
			{
				// Long option; no other options appear after "--"
				if (argv[t][2])
					options_long( argv[t]+2 );
				else
					do_options = false;
			}
			else
				options_short( argv[t]+1 );
			continue;
		}

		// Check for wildcard first
		if (!HasWildCard)
		{
			if (_Verbose)
				ERRPRINTF1( "FART: wild_card=\"%s\"\n", argv[t] );
			strcpy( WildCard, argv[t] );
			HasWildCard = true;
			continue;
		}

		// Check for find_string next
		if (!FindLength)
		{
			if (_Verbose)
				ERRPRINTF1( "FART: find_string=\"%s\"\n", argv[t] );
			FindLength = strlen(argv[t]);
			memcpy( FindString, argv[t], FindLength+1 );
			continue;
		}

		// Check for replace_string next
		if (!ReplaceLength)
		{
			if (_Verbose)
				ERRPRINTF1( "FART: replace_string=\"%s\"\n", argv[t] );
			ReplaceLength = strlen(argv[t]);
			memcpy( ReplaceString, argv[t], ReplaceLength+1 );
			continue;
		}

		ERRPRINTF1( "Error: redundant argument \"%s\".\n", argv[t] );
		_Help = true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int findtext_file_path( const char* dir, const char* file )
{
	char _path[_MAX_PATH];
	strcpy(_path,dir);
	strcat(_path,file);

	if (_Names)
	{
		const char *buf = findtext_line(file);
		if (buf)
		{
			// file name contains pattern; print filename
			TotalFileCount++;
			TotalFindCount++;
			puts(_path);
		}
	}
	else
	{
		// check file contents
		findtext_file( _path );
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int fart_file_path( const char* dir, const char* file )
{
	char *_path = strdup2(dir,file);

	if (_Names)
	{
		const char *buf = fart_line(file,fart_buf);
		if (buf)
		{
			TotalFileCount++;

			char *newpath = strdup2(dir,buf);
			if (_Preview || rename( _path, newpath )==0)
			{
				// Filename was changed
				TotalFindCount++;
				printf( __filename_rename, _path, buf);
			}
			else
			{
				ERRPRINTF2("Error: could not rename %s to %s\n", _path, buf );
				// CVS?
			}
			free(newpath);
		}
	}
	else
	{
		// find and replace in file contents
		fart( _path );
	}

	free(_path);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
	parse_options( argc, argv );

	if (_Help || !HasWildCard)
	{
		// S/He obviously needs some help
		DumpHelp();
		return -1;
	}

	if (!FindLength)
	{
		// FIND-mode: search for files matching the wildcard
		int i = for_all_wildcards( WildCard, &print_files );
		if (!_Quiet)
			printf("Found %i file(s).\n",i);
		return i;
	}

#ifdef _WIN32
	if (_Binary)
	{
		// Switch stdin/out to binary mode
		_setmode( _fileno(stdin), _O_BINARY );
		_setmode( _fileno(stdout), _O_BINARY );
	}
#endif

	TotalFileCount = TotalFindCount = 0;

	// Warn for conflicting options
	if (_Count && _Numbers)
		ERRPRINTF( "Warning: conflicting options: --line-number, --count\n" );

	// Expand c-style character constants
	if (_CStyle && FindLength)
	{
		FindLength = cstyle(FindString);
		if (_Verbose)
			ERRPRINTF1( "FART: actual find_length=%i\n", FindLength );
	}
	if (_CStyle && ReplaceLength)
	{
		ReplaceLength = cstyle(ReplaceString);
		if (_Verbose)
			ERRPRINTF1( "FART: actual replace_length=%i\n", ReplaceLength );
	}

	// Case insensitive: we compare in lower case
	if (_IgnoreCase && FindLength)
		strlwr(FindString);									// FIXME: memlwr

	// OPTIMIZE: Check for redundant FART (where find_string==replace_string)
	if (ReplaceLength && FindLength==ReplaceLength)
	{
		if (!_IgnoreCase && memcmp(FindString,ReplaceString,FindLength)==0)
		{
			ERRPRINTF( "Warning: strings are identical.\n");
			ReplaceLength = 0;								// 'grep' mode
		}
	}

	if (!ReplaceLength)
	{
		// GREP-mode
		if (strcmp( WildCard, "-" )==0)
		{
			// Find text in stdin
			// If asked to check filenames, just return 0 (pointless)
			int count = _Names?0:_findtext( stdin, NULL );
			if (!_Quiet)
				printf( "Found %i occurence(s).\n", count );
			return count;
		}

		// Find text in files
		for_all_wildcards( WildCard, &findtext_file_path );
		if (!_Quiet)
			printf( "Found %i occurence(s) in %i file(s).\n", TotalFindCount, TotalFileCount);

		return TotalFindCount;
	}


	// FART-mode

	memcpy( ReplaceStringLwr, ReplaceString, ReplaceLength+1 );
	strlwr( ReplaceStringLwr );							// FIXME: memlwr

	if (_AdaptCase)
	{
		if (_IgnoreCase)
		{
//			memcpy( ReplaceStringLwr, ReplaceString, ReplaceLength+1 );
//			strlwr( ReplaceStringLwr );							// FIXME: memlwr
			memcpy( ReplaceStringUpr, ReplaceString, ReplaceLength+1 );
			strupr( ReplaceStringUpr );							// FIXME: memlwr
			// We now have 3 strings: Lower, Mixed and Upper
		}
		else
		{
			// OPTIMIZE: We only need to adapt the replace_string once
			int i = analyze_case(FindString,FindLength);
			if (i==-1)
				strlwr(ReplaceString);							// FIXME: memlwr
			else
			if (i==1)
				strupr(ReplaceString);							// FIXME: memlwr
			if (i && _Verbose)
				ERRPRINTF1( "FART: actual replace_string=\"%s\"\n", ReplaceString );
			_AdaptCase = false;
		}
	}

	// OPTIMIZE: double-check to see whether anything really changed
	if (_IgnoreCase && FindLength==ReplaceLength)
		_DoubleCheck = memcmp(ReplaceStringLwr,FindString,FindLength)==0;

	if (strcmp( WildCard, "-" )==0)
	{
		// FART in stdin/stdout
		// If asked to check filenames, just return 0 (pointless)
		int count = _Names?0:_fart( stdin, stdout, NULL );
		if (!_Quiet)
			printf( "Replaced %i occurence(s).\n", count );
		return count;
	}

	if (_CVS && _Names)
	{
		ERRPRINTF("Error: renaming version controlled files would destroy their history\n");
		return -3;
	}
	
	if (_Binary && !_Preview)
	{
		// If the size changes, binary files will very likely stop working
		if (FindLength!=ReplaceLength && !_Backup)
		{
			ERRPRINTF( "Error: too dangerous; must specify --backup" );
			return -2;
		}
		// Warn about FART'ing binary files
		ERRPRINTF( "Warning: fart may corrupt binary files\n" );
	}

	_Backup = true;				// FIXME for now (unsafe)

	for_all_wildcards( WildCard, &fart_file_path );
	if (!_Quiet)
		printf( "Replaced %i occurence(s) in %i file(s).\n", TotalFindCount, TotalFileCount);

	return TotalFindCount;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
