#ifdef WINDOWS
    #include "stdafx.h"
#endif
#include "command.h"
using namespace std;

vector<string> arg_vec;
deque<string> commandList;
string command;
map<string, string> env;
extern char * hPath;
extern pid_t recentPid;
extern pid_t waitingPid;
extern int status;
extern bool hasAmp;
extern bool xFlag;
char * dPath;
bool isExit;

bool isWord(string s){
    int count = 0;
    for (char c : s){
        if (isalpha(c)){
            count++;
        }
    }
    if (count == 0){
        return false;
    }
    else {
        return true;
    }
}

bool built_in_command(string s){
    if (s == "show" || s == "set" || s == "unset" || s == "export" || s == "unexport" || s == "environ" || s == "chdir" || s == "environ" || s == "wait" || s == "exit" || s == "clr" || s == "dir" || s == "echo" || s == "help" || s == "pause" || s == "history" || s == "repeat" || s == "kill"){
        return true;
    }
    else {
        return false;
    }
}

void addCommand(string s){
    if (commandList.size() >= 100){
        commandList.pop_front();
        commandList.push_back(s);
    }
    else{
        commandList.push_back(s);
    }
}

void parse_command(string c){

    istringstream iss(c);
    string result;
    isExit = false;
    while (iss >> result){
        string::iterator itb;
        string::iterator ite;
        itb = result.begin();
        ite = itb + result.length() -1;
        if (*itb == '"'){
            result.erase(itb);
        }
        if (result[result.length()-1] == '"'){
            result.pop_back();
        }
        if (result == "$$"){
            std::string intToString = std::to_string(getpid());
            result = intToString;
            if (xFlag)
                cout << "$$ is " << result << endl;
        } else if (result == "$?"){
            std::string intToString = std::to_string(status);
            result = intToString;
            if (xFlag)
                cout << "$? is " << result << endl;
        } else if (result == "$!"){
            std::string intToString = std::to_string(waitingPid);
            result = intToString;
            if (xFlag)
                cout << "$! is " << result << endl;
        }
        if (result[0] == '$'){
            string::iterator it;
            it = result.begin();
            result.erase(it);
            map<string, string>::iterator iter;
            iter = env.find(result);
            if (iter != env.end()){
                result = iter->second;
            }
        }
        arg_vec.push_back(result);
    }
}


int exe(vector<string> arg_vec){
    // helper method to add the command to the command history list
    // keeps track of the last 100 commands.
    addCommand(command);

    string com = arg_vec[0];

    //kill
    if (com == "kill"){
        // kill command must take either a total of 2 or 3 arguments
        if (arg_vec.size() != 3 && arg_vec.size() != 2){
            perror("Command Error");
            return Done;
        }
        // if argument size is 2
        else if (arg_vec.size() == 2){
            // retrieves 2nd argument
            string com2 = arg_vec[1];
            int pd;
            istringstream iss(com2);
            // if the extraction to an int is successful (if it is an int)
            if (iss >> pd){
                kill (pd, SIGKILL);
                return Done;
            }
            // if the second command is not an int
            else {
                perror("Command Error");
                return Done;
            }
        }
        else {
            // retrieves 2nd and 3rd argument
            string com2 = arg_vec[1];
            string com3 = arg_vec[2];
            // cast the second command to a char array to check if the first char is a '-'
            char* com2Char = const_cast<char*>(com2.c_str());
            // if not '-' return error
            if (*com2Char != '-'){
                perror("Command Error");
                return Done;
            }
            // if the flag is present
            else {
                int sid;
                int pd;
                com2Char++;
                // wrap the char and string in istringstreams to be extracted into ints. 
                istringstream iss(com2Char);
                iss >> sid;
                istringstream iss2(com3);
                iss2 >> pd;
                // kill with signal # sid
                kill(pd, sid);
                return Done;
            }
        }
    }

    // wait (Test this)
    if (com == "wait"){
        if (arg_vec.size() != 2){
            perror("Command Error");
            return Done;
        }
        else {
            int iD;
            string com2 = arg_vec[1];
            if (com2 == "-1"){
                waitpid(recentPid, NULL, 0);
            }
            istringstream iss(com2);
            iss >> iD;
            pid_t pid = iD;
            waitpid(pid, NULL, 0);
            return Done;
        }
    }

    // pause
    if (com == "pause"){
        // pause must only have one argument
        if (arg_vec.size() == 1){
            return Pause;
        }
        // return command error if not
        else {
            perror("Command Error");
            return Done;
        }
    }

    // help
    if (com == "help"){

        pid_t pid;
        // if pid is less than 0 there was a fork error
        if (pid < 0)
            perror("Fork Error");
        // child loop after forking
        else if ((recentPid=pid = fork()) == 0) {
            fflush(stdout);
            // executes vi to read the man page text file
            execlp("vi", "vi", "readme",0);
            perror("execvp error");
            exit(1);
          // for parent loop
        } else{
            // if the command is a foreground command
            if (!hasAmp){
                waitpid(pid, &status, 0);
            }
            /* if the command is a background command set the waitingPid to the pid of the process. 
               used for the pause command */
            else waitingPid = pid;
        }

        return Done;
    }

    // dir command
    if (com == "dir"){
        // use of dirent.h to list contents of the directory of the specific path
        DIR *dir;
        struct dirent *ent;
        // retrieves the current working directory. This is in sync with the actual Shell 
        char* pathh = getenv("PWD");
        if ((dir = opendir(pathh)) != NULL) {
            /* print all the files and directories within directory */
            while ((ent = readdir(dir)) != NULL) {
                printf("%s\n", ent->d_name);
            }
            closedir(dir);
        }
        else {
            /* could not open directory */
            perror("Open Failure");
            return Done;
        }
    }

    // chdir
    if (com == "chdir"){
        // gets the current working directory
        char * pathIs = getenv("PWD");
        // if chdir is by itself, change to the "home" directory
        if (arg_vec.size() == 1){
            // string mutation
            string putPath = "PWD=";
            string s(hPath);
            putPath += s;
            // sets the working directory as the "home" directory
            setenv("PWD", s.c_str(), 1);
        }
        // if chdir has 2 commands
        else if (arg_vec.size() == 2){
            // set up an iterator pointing to the beginning of the string. 
            string::const_iterator it = arg_vec[1].begin();
            // if the command is xssh>> chdir .
            if (arg_vec[1] == "."){
                return Done;
            }
            // if the command is xssh>> chdir ..
            else if (arg_vec[1] == "..") {
                string cutPath(pathIs);
                // if the current working directory is at the root do return an error.
                if (cutPath.length() == 1){
                    perror("Path Error");
                    return Done;
                }
                int count = 0;
                // iterate from the back of the string to curtail the end until '/' appears
                for (std::string::reverse_iterator rIter = cutPath.rbegin(); rIter != cutPath.rend(); ++rIter){
                    // delete the first character no matter what. This is implemented incase the first character is a '/'
                    if (count == 0){
                        cutPath.erase(--(rIter.base()));
                        count++;
                        // continue the next iteration
                        continue;
                    }
                    if (*rIter != '/'){
                        cutPath.erase(--(rIter.base()));
                    }
                    // if the character is '/'
                    else {
                        break;
                    }
                }
                string stringIs = "PWD=";
                stringIs += cutPath;
                // set the current working directory 
                setenv("PWD", cutPath.c_str(), 1);
            }
            else if (*it == '/'){
                string stringIs = "PWD=";
                stringIs += arg_vec[1];
                // set the current working directory
                setenv("PWD", arg_vec[1].c_str(), 1);
            }
            else {
                string changeString(pathIs);
                string stringIs = "PWD=";
                string::reverse_iterator rI = changeString.rbegin();
                if (*rI != '/'){
                    changeString += "/";
                }
                changeString += arg_vec[1];
                stringIs += changeString;
                setenv("PWD", changeString.c_str(), 1);
            }
        }
        else {
            perror("Command Error");
            return Done;
        }
    }

    // repeat
    if (com == "repeat"){
        // if repeat default
        if (arg_vec.size() == 1 && commandList.size() > 2){
            // set command to the previous command
            // 2nd to the last on the command history due to the last command at this point being "repeat"
            command = commandList[commandList.size() - 2];
            return Repeat;
        }
        // if not default repeat
        else if (arg_vec.size() == 2){
            int dCount = 0;
            // checks every char in the string to see if the string is a number
            for (char c : arg_vec[1]){
                if (isdigit(c)){
                    dCount++;
                }
            }
            // if the string is indeed a number
            if (arg_vec[1].length() == dCount){
                int a;
                // wrap the string indicating a number in a istringstream to be extracted into an int format
                istringstream iss(arg_vec[1]);
                iss >> a;
                // -1 here because the range of positions in commandList is from 0 to 99. The range of positions indicated by the "history" command is from 1 to 100.
                command = commandList[a - 1];
                return Repeat;

            }
            // if the string is not a number the argument is not in the correct format.
            else {
                perror("Command Error");
                return Done;
            }
        }
        else {
            perror("Command Error");
            return Done;
        }
    }

    // history
    if (com == "history"){
        if (arg_vec.size() == 1){
            for (int i = 0; i < commandList.size(); i++){
                cout << i + 1 << ". " << commandList[i] << endl;
            }
        }
        else{
            // to check if number passed is actually a number
            int countr = 0;
            for (char c : arg_vec[1]){
                if (isdigit(c)){
                    countr++;
                }
            }
            // the second argument is a number
            if (arg_vec[1].length() == countr){
                int c;
                istringstream conv(arg_vec[1]);
                conv >> c;
                int limit = commandList.size() - c;
                // prints out the last c comments
                for (int i = limit; i < commandList.size(); i++){
                    cout << i + 1 << ". " << commandList[i] << endl;
                }
            }
            else{
                cout << "Invalid Argument (history number not specified)" << endl;
                return Done;
            }
        }
    }

    // show
    if (com == "show"){
        map<string, string>::iterator iter;
        // show must take more than one argument
        if (arg_vec.size() == 1){
            perror("No Variable");
            return Done;
        }
        // if only need to show one variable
        if (arg_vec.size() == 2){
            iter = env.find(arg_vec[1]);
            if (iter == env.end()){
                cout << "Variable " << arg_vec[1] << " Not Defined" << endl;
                return Done;
            }
            else {
                cout << iter->first << "= " << iter->second << endl;
                return Done;
            }
        }
        // more than one variable
        else {
            for (int a = 1; a < arg_vec.size(); a++){
                // find the variable
                iter = env.find(arg_vec[a]);
                // if the variable isn't set
                if (iter == env.end()){
                    cout << arg_vec[a] << "= NULL" << endl;
                    return Done;
                }
                // dispaly the variable with its value
                else {
                    cout << iter->first << "= " << iter->second << endl;
                    return Done;
                }
            }
        }
    }

    // environ
    if (com == "environ"){
        // if there are no environment variables
        if (env.size() < 1){
            perror("No Variable");
            return Done;
        }
        for (map<string, string>::iterator it = env.begin(); it != env.end(); ++it){
            cout << it->first << "=" << it->second << '\n';
        }
    }
    // clr
    if (com == "clr") {
        // ANSI/VT100 Terminal Control Escape Sequences
        printf("\033[H\033[J");
    }

    // *unset W1, unexport W, *exit I, wait I, *echo <comment>
    if (arg_vec.size() == 2){
        string com = arg_vec[0];
        string com2 = arg_vec[1];
        map<string, string>::iterator iter;
        string::const_iterator it = com2.begin();
        // exit
        if (com == "exit" && isdigit(*it)){
            int value;
            istringstream conv(com2);
            conv >> value;
            // creating a flag to tell that the shell should exit
            isExit = true;
            return value;
        }
        // echo
        if (com == "echo"){
            // simple standard outstream
            cout << com2 << endl;
            return Done;
        }
        // unset
        if (com == "unset"){
            iter = env.find(com2);
            // if the variable specified is non-existent
            if (iter == env.end()){
                perror("No Variable");
                return Done;
            }
            else{
                // delete from the map and unset the variable
                env.erase(com2);
                unsetenv(com2.c_str());
                return Done;
            }
        }
        // unexport
        if (com == "unexport"){
            iter = env.find(com2);
            if (iter == env.end()){
                perror("No Variable");
                return Done;
            }
            else{
                env.erase(com2);
                unsetenv(com2.c_str());
                return Done;
            }
        }
    }

    // set, export
    if (arg_vec.size() == 3){
        string com = arg_vec[0];
        string com2 = arg_vec[1];
        string com3 = arg_vec[2];
        map<string, string>::iterator iter;

        // set
        if (com == "set"){
            iter = env.find(com2);
            // if the variable to set doesn't exist insert a new variable
            if (iter == env.end()){
                env.insert(std::pair<string, string>(com2, com3));
            }
            // if the variable exists overwrite the variable with the new value
            else {
                iter->second = com3;
            }
            setenv(com2.c_str(), com3.c_str(), 1);
        }

        // export
        if (com == "export"){
            char* var;
            const char* varString = com2.c_str();
            var = getenv(varString);
            // if the var exists
            if (var != NULL){
                env.insert(std::pair<string, string>(com2, com3));
                return Done;
            }
            else {
                perror("No Variable");
                return Done;
            }

        }
    }
    return Done;
}