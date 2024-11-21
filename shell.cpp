#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <string>
#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

string getShelPrompt();

int main () {
    // save original stdin and stdout 
    int original_stdin = dup(STDIN_FILENO);   // Save the original stdin
    int original_stdout = dup(STDOUT_FILENO); // Save the original stdout
    // save previous current working directory
    char prev_cwd[1024];
    getcwd(prev_cwd, sizeof(prev_cwd));
    vector<int> background_pids; // initialize vector to store background pids
    // we will check the status of these pids at every input prompt iteration
    for (;;) {
        // check if any background processes have finished
        for (size_t i = 0; i < background_pids.size(); i++) {
            int status;
            int pid = waitpid(background_pids[i], &status, WNOHANG);
            if (pid == background_pids[i]) {
                // remove pid from vector
                background_pids.erase(background_pids.begin() + i);
                // print out pid and exit status
                cout << "[" << pid << "] " << "Done" << " " << status << endl;
            } else if (pid == -1) {
                perror("waitpid");
                return 1;
            }
        }
        // need date/time, username, and absolute path to current dir
        string prompt = getShelPrompt();
        cout << YELLOW << prompt << NC << " ";
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }

        // print out every command token-by-token on individual lines
        // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }

        // for each command in token.commands
        for (size_t i = 0; i < tknr.commands.size(); i++){
            if (tknr.commands[i]->args[0] == "cd") {
                // if cd is the command do not fork
                // get current working directory to save to prev_cwd
                char curr_cwd[1024];
                getcwd(curr_cwd, sizeof(curr_cwd));
                // check if cd arg is -
                if (tknr.commands[i]->args[1] == "-") {
                    // change to previous directory
                    if (chdir(prev_cwd) != 0) {
                        perror("Failed to change directory");
                        return 1;
                    }
                    continue;
                }
                if (chdir(tknr.commands[i]->args[1].c_str()) != 0) {
                    perror("Failed to change directory");
                    return 1;
                }
                // update prev_cwd
                strcpy(prev_cwd, curr_cwd);
                continue;
            }
            vector<string> cmd_args_str = tknr.commands[i]->args;
            // fork to create child
            pid_t pid;
            int fd[2];
            if (pipe(fd) == -1) {  // error check
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            pid = fork();  // create child process
            FILE *output_file_obj;
            FILE *input_file_obj;
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }
            if (pid == 0) {  // if child, exec to run command
                if (i < tknr.commands.size() - 1) {
                    dup2(fd[1], STDOUT_FILENO); // redirect output to write end of pipe
                }

                close(fd[0]); // close the read end of the pipe
                // check if input redirection is present
                bool input_redir = tknr.commands[i]->hasInput();
                // cerr << "input_redir: " << input_redir << endl;
                // check if output redirection is present
                bool output_redir = tknr.commands[i]->hasOutput();
                // cerr << "output_redir: " << output_redir << endl;
                if (input_redir){
                    // get input file name
                    string input_file = tknr.commands[i]->in_file;
                    input_file_obj = fopen(input_file.c_str(), "r");
                    // open(...)
                    // close(fd[])
                    // also include fstream
                    if (input_file_obj == NULL) {
                        perror("Failed to open file");
                        return 1;
                    }

                    // Get the file descriptor using fileno
                    int fd_in = fileno(input_file_obj);
                    // redirect stdin to input file
                    dup2(fd_in, STDIN_FILENO);
                } 
                if (output_redir){
                    string output_file = tknr.commands[i]->out_file;
                    output_file_obj = fopen(output_file.c_str(), "w");
    
                    if (output_file_obj == NULL) {
                        perror("Failed to open file");
                        return 1;
                    }

                    // Get the file descriptor using fileno
                    int fd_out = fileno(output_file_obj);
                    // redirect stdout to output file
                    dup2(fd_out, STDOUT_FILENO);
                }
                // change to run multiple args
                // convert vector<string> to vector of char*
                vector<char*> args;
                // char** args = new char*[cmd_args_str.size() + 1];
                for (size_t j = 0; j < cmd_args_str.size(); ++j ) {
                    args.push_back(strdup(cmd_args_str[j].c_str()));
                }
                args.push_back(NULL); // Add a NULL pointer at the end.
                
                if (execvp(args[0], args.data()) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }
            }
            else {  // if parent, wait for child to finish
                dup2(fd[0], STDIN_FILENO); // redirect input to read end of pipe
                close(fd[1]); // close the write end of the pipe
                int status = 0;
                int flag = 0; // default flag to wait for child
                // flag using WNOHANG man pages if & in command
                if (tknr.commands[i]->isBackground()) {
                    flag = WNOHANG; // don't wait for child
                    background_pids.push_back(pid); // add pid to vector
                    cerr << "[" << pid << "]" << endl; // print pid to cerr
                }
                if (i == tknr.commands.size() - 1) {
                    waitpid(pid, &status, flag);
                }
                if (status > 1) {  // exit if child didn't exec properly
                    exit(status);
                }
            }
            // if (tknr.commands[i]->hasInput() && input_file_obj != NULL) {
            //     // close input file
            //     fclose(input_file_obj);
            // }
            // if (tknr.commands[i]->hasOutput() && output_file_obj != NULL) {
            //     // close output file
            //     fclose(output_file_obj);
            // }
        }
        // Reset the input and output file descriptors of the parent.
        // overrite in/out with what was saved
        dup2(original_stdin, STDIN_FILENO);
        dup2(original_stdout, STDOUT_FILENO);
    }
}


string getShelPrompt(){
    // print out shell prompt
    // follow format Sep 23 18:31:46 user:/home/user$
    // time() + ctime() to get date and time
    time_t now = time(0);
    char* dt = ctime(&now);
    dt[24] = '\0';  // remove newline from ctime() output
    // copy dt to a string, then remove the newline
    string date_time = dt;
    // remove weekday and year from date_time
    date_time = date_time.substr(4, date_time.length() - 4 - 5);
    string user = getenv("USER");
    // get working directory
    // we cannot use getenv because we can change directory
    // and the shell prompt will not update
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    string prompt = date_time + " " + user + ":" + cwd + "$";
    return prompt;
}
