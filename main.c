// Compiler by Ansh Sharma 
// 11th December 2023

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define BUFFERSIZE 1024
#define TOKENBUFFERSIZE 64
#define TOKENDELIMITER " \t\r\n\a"

int minicshell_cd(char **args);
int minicshell_help(char **args);
int minicshell_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char**) = {
    &minicshell_cd,
    &minicshell_help,
    &minicshell_exit
};

int minicshell_number_of_builtins(){
    return sizeof(builtin_str) / sizeof(builtin_str[0]);
}

int minicshell_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr,"MiniCShell: expected arguments to \"cd\" \n");
    } else{
        if(chdir(args[1]) != 0){
            perror("Could not change directory");
        }
    }
    return 1;
}

int minicshell_help(char **args){
    int i;
    printf("Ansh Sharma's Mini C shell \n");
    printf("Type program names and arguments and hit enter. \n");
    printf("The following commands are built in: \n");
    
    for(int i = 0;i < minicshell_number_of_builtins();i++){
        printf(" %s\n",builtin_str[i]);
    }
    printf("Use the man command for information about other commands. \n");
    return 1;
}

int minicshell_exit(char **args){
    return 0;
}

char* minicshell_read_line(void){
    int bufsize = BUFFERSIZE;
    int position = 0;
    char* buffer = malloc(sizeof(char) * bufsize);
    int c;
    
    if(!buffer){
        fprintf(stderr, "MiniCShell:allocation error \n");
        exit(EXIT_FAILURE);
    }
    
    while(1){
        c = getchar();
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;
        
        if(position >= bufsize){
            bufsize += BUFFERSIZE;
            buffer = realloc(buffer,bufsize);
            if(!buffer){
                fprintf(stderr, "MiniCShell:allocation error \n");
                exit(EXIT_FAILURE);
            }
        }
    }
}
char **minicshell_tokenize(char *line){
    int bufsize = TOKENBUFFERSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    if(!tokens){
        fprintf(stderr, "MiniCShell:allocation error \n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, TOKENDELIMITER);
    while(token!= NULL){
        tokens[position] = token;
        position++;
        
        if(position >= bufsize){
            bufsize += TOKENBUFFERSIZE;
            tokens = realloc(tokens,bufsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, "MiniCShell:allocation error \n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOKENDELIMITER);
    }
    tokens[position] = NULL;
    return tokens;
}
int minicshell_launch(char **args){
    pid_t pid,wpid;
    int status;
    
    pid = fork();
    if(pid == 0){
        //child process
        if(execvp(args[0],args) == -1){
            perror("Error Running Command");
        }
        exit(EXIT_FAILURE);
    }else if(pid<0){
        //error forking
        perror("Error Executing");
    }
    else{
        //parent process
        do{
            wpid = waitpid(pid,&status,WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int minicshell_execute(char **args){
    int i;
    
    if(args[0] == NULL){
        return 1;
    }
    for(int i = 0;i<minicshell_number_of_builtins();i++){
        if(strcmp(args[0],builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return minicshell_launch(args);
}

void minicshell_loop(void){
    char *line;
    char **args;
    int status;
    do{
        printf("MiniCShell> ");
        line = minicshell_read_line();
        args = minicshell_tokenize(line);
        status = minicshell_execute(args);
        
        free(line);
        free(args);
    }while(status);
}


int main (int arg, char **argv){
    minicshell_loop();
    return 0;
}
