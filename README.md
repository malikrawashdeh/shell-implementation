# Shell Implementation

This project is a custom Linux shell implementation designed to mimic the functionality of the Bourne Again Shell (Bash). It allows users to execute commands, handle input/output redirection, manage processes, and utilize command pipelines.

## Features

- **Custom Prompt**:
  - Displays the current username, date/time, and the absolute path of the working directory.

- **Command Execution**:
  - Supports executing standard Linux commands like `ls`, `echo`, `cat`, and more using `fork()` and `execvp()`.

- **Input/Output Redirection**:
  - Redirect standard input and output to files using `<` and `>` operators.

- **Command Pipelining**:
  - Allows combining multiple commands with `|`, enabling interprocess communication.

- **Background Processes**:
  - Run commands in the background using `&` and manage them to prevent zombie processes.

- **Directory Management**:
  - Includes commands like `cd`, `pwd`, and `cd -` to navigate and manage the current working directory.

- **Support for Quotes**:
  - Handle single (`'`) and double (`"`) quotes for arguments with whitespace or special characters.

## How to Run

1. **Compile the Project**:
   Use the following command to compile the project:
   ```bash
   g++ -o shell src/*.cpp
   ```
2. **Start the Shell**: Run the compiled executable:
   ```bash
   ./shell
   ```
3. **Use the Shell**: Enter commands as you would in a standard Linux shell.

   
## Example Commands
- Basic Command:
  ```bash
  ls -l
  ```
- Input Redirection:
  ```bash
  grep "text" < file.txt
  ```
- Output Redirection:
  ```bash
  echo "Hello World" > output.txt
  ```
- Command Pipeline:
  ```bash
  ps aux | grep bash | awk '{print $1}'
  ```
- Background Process:
  ```bash
  sleep 5 &
  ```

  ## Lessons Learned

Through this project, I explored:

- Implementing process management with `fork()` and `execvp()`.
- Handling pipes and interprocess communication.
- Managing input/output redirection with `dup2()`.
- Building a robust shell interface with features like custom prompts and command parsing.

Feel free to explore and modify the code to experiment further!

## Future Enhancements

- Implementing tab completion for commands and file paths.
- Adding a command history feature to navigate previous commands with the arrow keys.
- Extending support for more advanced Bash features.

Let me know if you need further refinements!
