# FART

![GitHub release](https://img.shields.io/github/v/release/lionello/fart-it)

Find And Replace Text command line utility. New & improved version of the well-known grep command, with advanced features such as: case-adaption of the replace string; find (& replace) in filenames.

```
Usage: fart [options] [--] <wildcard>[,...] [find_string] [replace_string]

Options:
 -h, --help          Show this help message (ignores other options)
 -q, --quiet         Suppress output to stdio / stderr
 -V, --verbose       Show more information
 -r, --recursive     Process sub-folders recursively
 -c, --count         Only show filenames, match counts and totals
 -i, --ignore-case   Case insensitive text comparison
 -v, --invert        Print lines NOT containing the find string
 -n, --line-number   Print line number before each line (1-based)
 -w, --word          Match whole word (uses C syntax, like grep)
 -f, --filename      Find (and replace) filename instead of contents
 -B, --binary        Also search (and replace) in binary files (CAUTION)
 -C, --c-style       Allow C-style extended characters (\xFF\0\t\n\r\\ etc.)
     --cvs           Skip cvs dirs; execute "cvs edit" before changing files
     --svn           Skip svn dirs
     --git           Skip git dirs (default)
     --remove        Remove all occurences of the find_string
 -a, --adapt         Adapt the case of replace_string to found string
 -b, --backup        Make a backup of each changed file
 -p, --preview       Do not change the files but print the changes
```
