//  Copyright (C) 1998-2001 Mondo Bizzarro BV.  All rights reserved.
//  
//  This file is part of MBFPL vrSS.
//  
//  MBFPL vrSS is distributed with NO WARRANTY OF ANY KIND.  No author or
//  distributor accepts any responsibility for the consequences of using it,
//  or for whether it serves any particular purpose or works at all, unless
//  he or she says so in writing.  Refer to the Mondo Bizzarro Free Public
//  License (the "License") for full details.
//  
//  Every copy of MBFPL vrSS must include a copy of the License, normally
//  in a plain ASCII text file named MBFPL.TXT.  The License grants you the
//  right to copy, modify and redistribute MBFPL vrSS, but only under certain
//  conditions, as described in the License.  Among other things, the License
//  requires that the copyright notice and this notice be preserved on all
//  copies.
//

//  FART <wc>					Show files that comply with <wc>

//  FART <wc> <s>				Show files, show lines containing <s>
//  FART -f <wc> <s>			Show files [count] containing <s>
//  FART -fq <wc> <s>			Show files containing <s>
//  FART -d <wc> <s>			Show files NOT containing <s>
//  FART -r <wc> <s>			Show all files, show lines NOT containing <s>

//  FART <wc> <s> <r>			Show files, replace <s> with <r>
//  FART -f <wc> <s> <r>		Show files [count], replace <s> with <r>
//  FART -r <wc> <s> <r>		Show files, remove lines containing <s> (ignores <r>)
//  FART -d <wc> <s> <r>		Show files, remove lines NOT containing <s> (ignores <r>)


#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <direct.h>

#ifdef VSS6
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <afxdisp.h>        // MFC OLE automation classes
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef VSS6
#define VERSION		"v1.72s"
#else
#define VERSION		"v1.72"
#endif

#define TEMPFILE "_temp.~fr"

static const char O_BACKUP = 'b';
static const char O_NOTHING = 'd';
static const char O_QUIET = 'q';
static const char O_HELP = 'h';
static const char O_IGNORECASE = 'i';
static const char O_ADAPTCASE = 'a';
static const char O_ONLYNAMES = 'f';
static const char O_LINENUM = 'l';
static const char O_SUBDIR = 's';
static const char O_WHOLEWORD = 'w';
static const char O_FILENAMES = 'n';

static const char O_REMLINE = 'r';
static const char O_KEEPLINE = 'k';
static const char O_CSTYLE = 'c';

bool	HasWildCard = false;
bool	DoFind = false;
bool	DoReplace = false;
bool	DoContent = true;

bool	DoBackup = false;
bool	DoNothing = false;
bool	Quiet = false;
bool	ShowHelp = false;
bool	IgnoreCase = false;
bool	OnlyNames = false; 
bool	LineNumbers = false;
bool	DoSubDir = false;
bool	DoRemLine = false;
bool	DoKeepLine = true;
bool	AdaptCase = false;
bool	WholeWord = false;
bool	CStyle = false;

bool	BOL=false;
bool	EOL=false;

#ifdef VSS6
#include "ssapi.h"
  static const char O_VSS = 'v';
  bool	UseVSS = false;
  IVSSDatabase vssdb;
  CString	vss_path = "\\\\crash\\vrss_db";
  CString	vss_user = "Lionello";
  CString	vss_pass = "******";
#endif


int		FindStringLength;
int		FileCount = 0;					// Number of files
int		FindCount = 0;					// Number of total occurences
//int	RSL;

#define MAXSTRING 1024

char	WildCard[MAXSTRING];
char	FindString[MAXSTRING];
char	ReplaceString[MAXSTRING];

char	FindString2[MAXSTRING];
char	ReplaceString2[MAXSTRING];

//char	AdaptString[MAXSTRING], FindStringUC[MAXSTRING], FindStringLC[MAXSTRING];
char	fart_buf[MAXSTRING], fart_buf2[MAXSTRING];

///////////////////////////////////////////////////////////////////////////////

inline bool _iswordchar( char c )
{
	return c=='_' || (c>='0'&&c<='9') || (c>='a'&&c<='z') || (c>='A'&&c<='Z');
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void DumpHelp()
{
	printf("Usage: FART [options] <wildcard>[;wildcard...] [find_string] [replace_string]\n");
	printf("\nOptions:\n");
	printf("\t%c\tCreate a backup of the original file\n", O_BACKUP );
	printf("\t%c\tIgnore case\n" , O_IGNORECASE );
	printf("\t%c\tAdapt case of replace_string\n" , O_ADAPTCASE );
	printf("\t%c\tMatch whole word only\n", O_WHOLEWORD );
	printf("\t%c\tChange file names instead of file content\n", O_FILENAMES );
	printf("\t%c\tRecurse subdrectories\n" , O_SUBDIR );
	printf("\t%c\tDisplay linenumbers\n" , O_LINENUM );
	printf("\t%c\tOnly display filenames\n" , O_ONLYNAMES );  
	printf("\t%c\tDummy mode, changes nothing\n" , O_NOTHING );  
	printf("\t%c\tRemove lines containing the find_string [CAUTION]\n" , O_REMLINE );  
	printf("\t%c\tRemove lines NOT containing the find_string [CAUTION]\n" , O_KEEPLINE );  
#ifdef VSS6
	printf("\t%c\tUse SourceSafe to check-out any file about to change\n" , O_VSS );  
#endif
	printf("\t%c\tShow this help screen (ignores all other parameters)\n" , O_HELP );
//	printf("\t%c\tAllow C-style extended characters (\\n\\t\\r)\n", O_CSTYLE );
	printf("\t%c\tQuiet\n" , O_QUIET );
	printf("\nOptions may be preceded with either / OR -\n\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool adaptcase( const char* in, char* out, int inl )
{
	int UC=0, LC=0;

	for (int t=0;t<inl;t++)
	{
		char uc = toupper(in[t]);
		char lc = tolower(in[t]);

		if (uc==lc)			// char is not alphabetic
		{
//			AdaptString[t]=0;
			continue;
		}

		if (uc==in[t])		// char is uppercase
		{
//			AdaptString[t]=1;
			UC++;
		}
		else				// char is lowercase
		{
//			AdaptString[t]=-1;
			LC++;
		}
	}

	if (UC==0 && LC==0)						// no alphabetic chars
		return false;

	// If all the chars are either upper- or lowercase
	// we can get rid of the 'unknown' cases

	if (UC==0)
	{
		strlwr(out);						// make them all lowercase
		return true;
	}

	if (LC==0)
	{
		strupr(out);						// make them all uppercase
		return true;
	}

	return false;

/*
	// Adapt the case of the 'out' string using 'AdaptString'
	for (t=0;t<inl;t++)
	{
		if (AdaptString[t]==0)			// char is not alphabetic
			continue;

		if (AdaptString[t]==1)			// char should be uppercase
			out[t] = toupper(out[t]);
		else							// char should be lowercase
			out[t] = tolower(out[t]);
	}*/
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Method that prints a string with CR/LF temporarily cut off

void puts_nocrlf( char *_buf )
{
	int nl=strlen(_buf);

	while (nl>0 && (_buf[nl-1]=='\r' || _buf[nl-1]=='\n')) nl--;

	char o = _buf[nl];
	_buf[nl] = '\0';
	puts(_buf);
	_buf[nl] = o;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

char* findtext_line( char* line )
{
	// Check file name
	if (IgnoreCase)
	{
		// Copy the text into an extra buffer (used just for comparison)
		strcpy(fart_buf2,line);
		line = fart_buf2;
		// Compare lowercase (FindString already is lowercase)
		strlwr(line);
	}

	char *t = strstr(line,FindString);

	// Check for word boundary
	if (t && WholeWord)
	{
		if (t>line && _iswordchar(t[-1]))
			t = NULL;
		else
		if (_iswordchar(t[FindStringLength]))
			t = NULL;
	}
	return t;
}

///////////////////////////////////////////////////////////////////////////////
// Find text in file
//  !DoKeepLine			Only show files NOT containing find_string
//  DoRemLine			Hide lines containing find_string

bool findtext( const char* in )
{
	int	this_find_count=0;					// Number of occurences in this file
	bool first = true;
	int ln=0;

	// Open file for reading
	FILE *f = fopen(in,"rt");
	if (!f)
	{
		printf("Error: Unable to open file %s\n", in );
		return false;
	}

	// Process input file while data available
	while (!feof(f))
	{
		char *b = fart_buf;
		ln++;
		if (!fgets( fart_buf, 1024, f ))
			break;

		char *t = findtext_line( fart_buf );

		if (t)
		{
			// find_string was found but we're only showing files without the find_string
			if (!DoKeepLine)
			{
				first = false;
				break;
			}

			// find_string was found but we're removing lines with the find_string
			if (DoRemLine)
				continue;
		}
		else
		{
			// find_string was not found
			if (!DoKeepLine)
				continue;

			if (!DoRemLine)
				continue;
		}

		this_find_count++;
		FindCount++;
			
		if (first)
		{
			first = false;
			if (Quiet && OnlyNames)
			{
				FileCount++;
				printf("%s\n", in );
				break;						// No need to continue
			}
			if (!OnlyNames && !Quiet)
			{
				// We're dumping the lines after this
				FileCount++;
				printf("%s :\n", in );
			}
		}

		if (OnlyNames)
			continue;

		if (LineNumbers)
			printf("[%4i]", ln );

		puts_nocrlf( fart_buf );
	}

	// Close file handle
	fclose(f);

	if (!DoKeepLine)
	{
		// Show files NOT containing find_string
		if (first)
		{
			FileCount++;
			printf("%s\n", in );
		}
	}
	else
	if (OnlyNames && !Quiet)
	{
		// Show filenames + count
		if (DoRemLine || !first)
		{
			FileCount++;
			printf("%s [%i]\n", in, this_find_count );
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FILE* open_tempfile()
{
	// Open temporary file for writing
	FILE *f2 = fopen(TEMPFILE,"wt");
	if (!f2)
		printf("Error: Unable to create temporary file\n" );
	return f2;
}

///////////////////////////////////////////////////////////////////////////////

int temp_fputs( const char* str, FILE* &f )
{
	if (!f) 
	{
		f = open_tempfile();
		if (!f)
			return -1;
	}
	return fputs(str,f);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Find and replace text in file (uses temporary file)

bool fart( const char* in )
{
	int	this_find_count=0;					// Number of occurences in this file
	bool first = true;
	bool replace = false;
	int ln=0;

	// Open original file for reading
	FILE *f1 = fopen(in,"rt");
	if (!f1)
	{
		printf("Error: Unable to open file %s\n", in );
		return false;
	}

	// Open temporary file for writing
	FILE *f2 = fopen(TEMPFILE,"wt");
	if (!f2)
	{
		printf("Error: Unable to create temporary file\n" );
		fclose(f1);
		return false;
	}

	// Process input file while data available
	while (!feof(f1))
	{
		char* b = fart_buf, *o=fart_buf;
		ln++;
		if (!fgets( fart_buf, 1024, f1 ))
			break;

//		fart_buf[strlen(fart_buf)-1]=0;
//		puts( fart_buf );

		if (IgnoreCase)
		{
			b = fart_buf2;
			strcpy( b, fart_buf );
			strlwr( b );
		}
		
		bool first_line=true;
		char* bp = b;
		while (1)
		{
			char *t = strstr(bp,FindString);

			// Check for word boundary
			if (t && WholeWord)
			{
				if (t>bp && _iswordchar(t[-1]))
					t = NULL;
				else
				if (_iswordchar(t[FindStringLength]))
					t = NULL;
			}

			if (!t)
			{
				// find_string not found
				if (DoKeepLine)
				{
					fputs( fart_buf+(bp-b), f2 );
					break;
				}
			}
			else
			{
				this_find_count++;
				FindCount++;
			}

			// First occurence in this file?
			if (first)
			{
				FileCount++;
				first = false;
				// Print the filename
				if (!OnlyNames && !Quiet)
					printf("%s\n", in );
			}

			// Remove lines containing the find_string
			if (DoRemLine)
			{
				replace = true;
				break;
			}

			// Don't replace find_string if DoKeepLine is NOT set
			if (!DoKeepLine)
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

			// First occurence in this line?
			if (first_line)
			{
				if (LineNumbers)
					printf("[%4i]", ln );
//				if (!Quiet)
//					puts_nocrlf(fart_buf);
				first_line = false;
			}

			bool RS2=false;

			// Adapt the replace_string to the actually found string
			if (AdaptCase)
				RS2 = adaptcase( fart_buf+(t-b), ReplaceString2, FindStringLength );

			// Put a '\0' just before the find_string
			*(fart_buf+(t-b)) = '\0';
			// Write the text before the find_string
			fputs( fart_buf+(bp-b), f2 );
			// Write the replace_string instead of the find_string
			fputs( RS2?ReplaceString2:ReplaceString, f2 );
			// Put the buffer-pointer right after the find_string
			bp = t + FindStringLength;

			// Something got replaced
			replace = true;
		}
	}

	// Close file handles
	fclose(f2);
	fclose(f1);

	if (replace && OnlyNames && !Quiet)
	{
		// Show filenames + count
		printf("%s [%i]\n", in, this_find_count );
	}

	if (replace && !DoNothing)
	{
		if (DoBackup)
		{
			char d[_MAX_DRIVE];
			char p[_MAX_DIR];
			char f[_MAX_FNAME];
			char e[_MAX_EXT], ext[_MAX_EXT+_MAX_EXT]=".~";
			_splitpath( in, d, p, f, e );
			if (e[0])
				strcat(ext,e+1);
			_makepath( fart_buf, d, p, f, ext );
			// Rename original file to backup-filename
			if (rename( in, fart_buf )!=0)
				printf( "Error: File %s is read only\n", in );
		}
		else
		{
			// Remove original file
			if (remove( in )!=0)
				printf( "Error: File %s is read only\n", in );
		}
		// Rename temporary file to original filename
		rename( TEMPFILE, in );
		// Remove temporary file (in case rename failed)
		remove( TEMPFILE );
	}
	else
	{
		// Nothing has changed; remove temporary file
		remove( TEMPFILE );
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool fart_filename( const char *dir, const char *filename )
{
	const char *b = filename;

	if (IgnoreCase)
	{
		strcpy( fart_buf2, filename );
		strlwr( fart_buf2 );
		b = fart_buf2;
	}

	fart_buf[0] = 0;
	const char *bp = b;
	while (1)
	{
		char *t = strstr(bp,FindString);

		// Check for word boundary
		if (t && WholeWord)
		{
			if (t>bp && _iswordchar(t[-1]))
				t = NULL;
			else
			if (_iswordchar(t[FindStringLength]))
				t = NULL;
		}

		// TODO
		if (!t)
			break;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Recurse through directories and search for files matching a wildcard

void recurse( const char *dir, const char* wc )
{
	_finddata_t fd;

//	printf("PATH %s%s\n", dir,wc);

	// Make full path wildcard
	char _path[_MAX_PATH];
	strcpy( _path, dir );
	strcat( _path, wc );

	long fh = _findfirst( _path, &fd );
	for (int lf=(int)fh;lf!=-1;lf=_findnext(fh,&fd))
	{
		// Do files now; process folders later
		if (fd.attrib & _A_SUBDIR)
			continue;

		// Make full path to file
		strcpy(_path,dir);
		strcat(_path,fd.name);

		if (DoReplace)
		{
			// Mode 1: Find and replace text (wildcard with find & replace strings)

			if (DoContent)
			{
				// Check file content
				fart( _path );
			}
			else
			{
				// Check file name
				fart_filename( dir, fd.name );
			}
			continue;
		}
		else
		if (DoFind)
		{
			// Mode 2: Find text (wildcard with find string)

			if (DoContent)
			{
				// Check file content
				findtext( _path );
			}
			else
			{
				// Check file name
				if (findtext_line(fd.name))
				{
					// Found; write file path
					puts( _path );
					// Count filename
					FileCount++;
				}
			}
			continue;
		}
		else
		{
			// Mode 3: Find files (just wildcard)

			// Count filename
			FileCount++;

			puts(_path);
		}
	}
	_findclose(fh);

	// Now we recurse into folders
	if (!DoSubDir)
		return;

	// Find all dirs (*.*)
	strcpy( _path, dir );
	strcat( _path, "*.*" );

	fh = _findfirst( _path, &fd );
	for (lf=(int)fh;lf!=-1;lf=_findnext(fh,&fd))
	{
		if (fd.attrib & _A_SUBDIR)
		{
			// Skip .
			if (!strcmp(fd.name,"."))
				continue;
			// Skip ..
			if (!strcmp(fd.name,".."))
				continue;

			// No chdir; this would mess up the command line
			strcpy(_path,dir);
			strcat(_path,fd.name);
			strcat(_path,"\\");
			recurse(_path,wc);
		}
	}
	_findclose(fh);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Parse a singlechar command line option

void parseoption( char os )
{
//	switch (tolower(os[1]))
	char opt = tolower(os);
	if (opt==O_BACKUP)
		DoBackup = true;
	else
	if (opt==O_IGNORECASE)
		IgnoreCase = true;
	else
	if (opt==O_FILENAMES)
		DoContent = false;
	else
	if (opt==O_SUBDIR)
		DoSubDir = true;
	else
	if (opt==O_LINENUM)
		LineNumbers = true;
	else
	if (opt==O_QUIET)
		Quiet = true;
	else
	if (opt==O_ONLYNAMES)
		OnlyNames = true;
	else
	if (opt==O_NOTHING)
		DoNothing = true;
	else
	if (opt==O_REMLINE)
		DoRemLine = true;
	else
	if (opt==O_KEEPLINE)
		DoKeepLine = false;
	else
	if (opt==O_ADAPTCASE)
		AdaptCase = true;
	else
	if (opt==O_WHOLEWORD)
		WholeWord = true;
	else
//	if (opt==O_CSTYLE)
//		CStyle = true;
//	else
#ifdef VSS6
	if (opt==O_VSS)
		UseVSS = true;
	else
#endif
	if (opt=='?' || opt=='h')
		ShowHelp = true;
	else
		printf("Error: Invalid option %c\n", opt );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
	char option_char = 0;

	// Parse all program arguments
	for (int t=1;t<argc;t++)
	{
		// Parse the options (if applicable)
		if (argv[t][0]=='/' || argv[t][0]=='-')
		{
			// Check for the correct option-identifier
			if (!option_char || option_char == argv[t][0])
			{
				option_char = argv[t][0];
				int stl = (signed)strlen(argv[t]);
				for (int tt=1;tt<stl;tt++)
					parseoption( argv[t][tt] );
				continue;
			}
		}

		// Check for wildcard first
		if (!HasWildCard)
		{
			strcpy( WildCard, argv[t] );
			HasWildCard = true;
			continue;
		}

		// Check for find_string next
		if (!DoFind)
		{
			strcpy( FindString, argv[t] );
			FindStringLength = strlen(FindString);
			DoFind = true;
			continue;
		}

		// Check for replace_string next
		if (!DoReplace)
		{
			strcpy( ReplaceString, argv[t] );
			strcpy( ReplaceString2, argv[t] );
			DoReplace = true;
			continue;
		}

		printf("Error: Ignoring \"%s\"\n", argv[t] );
//		ShowHelp = true;
	}

	// Print banner
	if (!Quiet)
		printf("\nFind And Replace Text  %s                             by Mondo Bizzarro BV\n\n",VERSION);

	// Warn for conflicting options
	if (DoRemLine && !DoKeepLine)
	{
		printf("Error: Conflicting options (%c and %c)\n", O_REMLINE, O_KEEPLINE );
		DoReplace = false;
	}

	// Warn for conflicting options
	if (OnlyNames && LineNumbers)
		printf("Error: Conflicting options (%c and %c)\n", O_ONLYNAMES, O_LINENUM );

	// Adjust options
	if (AdaptCase)
		 IgnoreCase = true;

	if (IgnoreCase && DoFind)
		strlwr( FindString );

#ifdef VSS6
	// Initialize VSS
	::OleInitialize(NULL);

	if (UseVSS)
	{
		if (vssdb.CreateDispatch("SourceSafe")==0)
		{
			printf("Error: SourceSafe server not found\n");
			UseVSS = false;
		}
		else
		{

			vssdb.Open(vss_path,vss_user,vss_pass);

		}
	}
#endif

	if (CStyle)
	{
		// TODO
//		find_and_replace( FindString, "\\n", "\n" );
	}

	if (HasWildCard && !ShowHelp)
	{
		// Process each substring in the wildcard (seperated by ;)
		char *b = WildCard;
		while (*b)
		{
			char *f = strchr( b, ';' );
			if (f) *f=0;
			char _d[_MAX_DRIVE];
			char _p[_MAX_DIR];
			char _f[_MAX_FNAME];
			char _e[_MAX_EXT];
			_splitpath( b, _d, _p, _f, _e );
			char path[256];
			char file[256];
			strcpy( path, _d );
			strcat( path, _p );
			strcpy( file, _f );
			strcat( file, _e );
			// Do the real thing right now!
			recurse(path,file);
			if (!f)
				break;
			b = f+1;
		}
		if (!Quiet)
		{
			// Show a nice summary
			if (FileCount==1)
			{
				if (FindCount==1)
					printf("Found one occurence in one file.\n",FindCount,FileCount);
				else
				if (FindCount)
					printf("Found %i occurences in one file.\n",FindCount,FileCount);
				else
					printf("Found one file.\n",FileCount);
			}
			else
			if (FileCount)
			{
				if (FindCount==1)
					printf("Found one occurence in %i files.\n",FileCount);
				else
				if (FindCount)
					printf("Found %i occurences in %i files.\n",FindCount,FileCount);
				else
					printf("Found %i files.\n",FileCount);
			}
			else
				printf("No files found.\n");
		}
	}
	else
		ShowHelp = true;

#ifdef VSS6
	::OleUninitialize();
#endif

	// Need help?
	if (ShowHelp)
		DumpHelp();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
