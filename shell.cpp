// Shell.cpp : Defines the entry point for the console application.
//
#ifdef WINDOWS
	#include "stdafx.h"
#endif

//#include <tclDecls.h>
#include "shell.h"
//#define _XOPEN_SOURCE

using namespace std;

extern vector<string> arg_vec;
extern deque<string> commandList;
extern string command;
extern map<string, string> env;
extern char* dPath;
extern pid_t recentPid;
extern bool isExit;
bool firstRun=true;
bool xFlag = false;
int dFlag = 0;
char* hPath;

// prompt for the toy shell
static const char*  promptString = "xssh>> ";
static volatile sig_atomic_t sig_caught = 0;
// instantiating different types of Signal Handlers
void(*int_handler)(int);
void(*abrt_handler)(int);
void(*alrm_handler)(int);
void(*hup_handler)(int);
void(*term_handler)(int);
void(*quit_handler)(int);
void(*stop_handler)(int);
void(*cont_handler)(int);
void(*usr1_handler)(int);
void(*usr2_handler)(int);

// absolute function for all signals
void sig_handler(int signum){
    // SIGINT for ctrl c
    if (signum == SIGINT){
        // re-register the signal handler after the signal is caught.
        int_handler = signal(SIGINT, sig_handler);
        // checks if there is a process by checking the recentPid value.
        if(recentPid>0)
            kill(recentPid, signum); // kills the signal with the signum value, in this case SIGINT or 2 for ctrl c
        cout << endl;
    }
    // Handling SIABRT
    if (signum == SIGABRT){
        abrt_handler = (SIGABRT, sig_handler);
    }
    // Hadling SIGTERM
    if (signum == SIGTERM){
        term_handler = signal(SIGTERM, sig_handler);
    }
    // Handling SIGALRM
    if (signum == SIGALRM){
        alrm_handler = signal(SIGALRM, sig_handler);
    }
    // Handling SIGQUIT
    if (signum == SIGQUIT){
        quit_handler = signal(SIGQUIT, sig_handler);
        // kills the last foreground process with the signal SIGQUIT
        kill(recentPid, signum);
        cout << endl;
    }
    // HAndling SIGCONT
    if (signum == SIGCONT){
        cont_handler = signal(SIGCONT, sig_handler);
        if(recentPid>0)
            kill(recentPid, signum);
        cout << endl;
    }
    // HAndling SIGSTOP
    if (signum == SIGTSTP){
        stop_handler = signal(SIGTSTP, sig_handler);
        if(recentPid>0)
            kill(recentPid, signum);
        cout << endl;
    }
    // SIGUSR1
    if (signum == SIGUSR1){
        usr1_handler = signal(SIGUSR1, sig_handler);
    }
    // SIGUSR2
    if (signum == SIGUSR2){
        usr2_handler = signal(SIGUSR2, sig_handler);
    }

    // SIGHUP
    if (signum == SIGHUP){
        hup_handler = signal(SIGHUP, sig_handler);
    }
}

// initialize the environment variables in the shell
void initialize(){
    // shell
    char* pat = getenv("PWD");
    string s(pat);
    env.insert(pair<string, string>("home", s));
    s+="/xssh";
    env.insert(pair<string, string>("shell", s));

    // Path
    char* pat2 = getenv("PATH");
    string s2(pat2);
    env.insert(pair<string, string>("PATH", s2));

    //Parent
    env.insert(pair<string, string>("parent", s));

    setenv("shell", s.c_str(), 1);
    setenv("home", s.c_str(), 1);
    setenv("parent", s.c_str(), 1);


}

// checking the flags to check for a file to parse the input from
bool checkInputParams(int argc, char * argv[]){
    bool hasParam = false;

    for (int i = 0; i < argc; i++){
        char fFlags[3];
        strcpy(fFlags, "-f");
        char xFlags[3];
        strcpy(xFlags, "-x");
        char dFlags[3];
        strcpy(dFlags, "-d");

        if (strcmp(fFlags, argv[i])==0){
            hasParam = true;
            int fd;
            pipe(&fd);
            pid_t pid;

            fd = open(argv[i+1], O_RDONLY);
            dup2(fd, 0); // dup into stdin

            close(fd);
            fflush(stdout);

            for (int j=i+2; j < argc; j++){
                int index = j-i-1;
                string keyEvn(argv[i]);
                string indexS = to_string(index);
                string valEvn= "$"+indexS;
                env.insert(pair<string, string>(keyEvn, valEvn));
            }
        }

        if (strcmp(xFlags, argv[i])==0){
            xFlag = true;
        }
    }
    firstRun = false;
    return hasParam;
}

int main(int argc, char * argv[])
{
    int sin_cpy = dup(0);
    hPath = getenv("PWD");
    // setting the sig_handler funtion as the goto function for all the signal handlers.
    int_handler = signal(SIGINT, sig_handler);
    abrt_handler = signal(SIGABRT, sig_handler);
    term_handler = signal(SIGTERM, sig_handler);
    alrm_handler = signal(SIGALRM, sig_handler);
    quit_handler = signal(SIGQUIT, sig_handler);
    cont_handler = signal(SIGCONT, sig_handler);
    stop_handler = signal(SIGTSTP, sig_handler);
    usr1_handler = signal(SIGUSR1, sig_handler);
    usr2_handler = signal(SIGUSR2, sig_handler);
    hup_handler = signal(SIGHUP, sig_handler);

    initialize();

    while (1){
        cin.clear();

        printf("%s", promptString);
        fflush(stdout);
        // firstRun utilized to restore the stdin after the redirection to read from the script file
        if (firstRun) {
            checkInputParams(argc, argv);
        } else {
            dup2(sin_cpy, 0);
        }
        getline(cin, command);
        // Utilizing label to be used for the repeat command.
        label:
        // if the input is empty
        if (command.length() < 1) {
            continue;
        }

        // helper method to parse the input string
        parse_command(command);

        // checks if the command is a built in command
        if (built_in_command(arg_vec[0])){
            int result = exe(arg_vec);
            // check if the input asked to exit
            if (isExit){
                return result;
            }
            // if the result is Pause
            if (result == Pause){
                // raises SIGSTOP to stop the
                cin.clear();
                string temp;
                getline(cin, temp);
                while (temp != "") {
                    getline(cin, temp);
                }
                continue;
            }
                // if the result is repeat
            else if (result == Repeat){
                arg_vec.clear();
                // utilize label to re run the mutated command
                goto label;
            }
                // if the result is done
            else if (result == Done){
                arg_vec.clear();
                continue;
            }
            // if the result is not an external command
        } else {
            // external command
            checkPipesNumber(arg_vec);
        }
        arg_vec.clear();
    }
    return 0;
}