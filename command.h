#ifdef WINDOWS
    #include "stdafx.h"
    #include <dirent.h>
#include <Windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <istream>
#include <vector>
#include <stdlib.h>
#include <map>
#include <deque>
#include <signal.h>
#include <curses.h>
#include <termcap.h>
#include <cstdlib>
#include <csignal>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>     /* Symbolic Constants */
#include <sys/wait.h>   /* Wait for Process Termination */
#include <errno.h>      /* Errors */
#include <sys/types.h>  /* Primitive System Data Types */
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

int exe(vector<string> vec);
bool built_in_command(string s);
void parse_command(string c);
bool isWord(string s);
void addCommand(string s);
bool built_in_command(string s);

enum commands{ Done, Pause, Not, Exit, Repeat};

enum errs{};