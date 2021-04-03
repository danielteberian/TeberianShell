#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define RL_BUFF_SIZE 1024
#define TK_BUFF_SIZE 64
#define TOK_DELIM " \t\r\n\a"

char *clr[2] = {"clear", NULL};

//FONT AND COLOR DEFINITIONS

#define BLUE	"\033[0;34m"
#define BOLD	"\e[1m"
#define CYAN	"\033[0;36m"
#define GREEN	"\033[0;32m"
#define INVERT	"\033[0;7m"
#define ITALICS	"\e[3m"
#define RED	"\033[0;31m"
#define RESET	"\e[0m"
#define YELLOW	"\033[0;33m"



//DECLARE FUNCTIONS

char *get_hist_path();
char *read_ln();
char **split_ln(char *);
char **split_pipe(char *);
char *trimws(char *);

int args_length(char **);
int hist_ln_cnt();
int tsh_cd(char **);
int tsh_exec(char **);
int tsh_exit(char **);
int tsh_grep(char **);
int tsh_help(char **);
int tsh_hist();
int tsh_launch(char **);
int tsh_pipe(char **);

void get_directory(char *);
void hist_input(char **, char *);
void pipe_hist_input(char *);
void print_tkn(char **);
void signalHandler();



int (*builtin_funcs[])(char **) = {&tsh_cd, &tsh_help, &tsh_exit, &tsh_hist, &tsh_grep, &args_length };

char *builtin_str[] = { "cd", "help", "exit", "history", "grep", "sizeof" };

int builtin_funcs_count()
{
	return sizeof(builtin_str) / sizeof(char *);
}



//PRINTS PATH FOR HISTORY CMD (DEFAULT IS HOME)
char *get_hist_path()
{
	static char file_path[128];
	strcat(strncpy(file_path, getenv("HOME"), 113), "/.tsh_hist");
	return file_path;
}


char *read_ln()
{
	int buffsize = RL_BUFF_SIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * buffsize);
	int c;

	if(!buffer)
	{
		fprintf(stderr, "%stsh: ALLOCATION ERROR%s\n", RED, RESET);
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		c = getchar();
		if (c == EOF || c == '\n')
		{
			buffer[position] = '\0';
			return buffer;
		}
		else
		{
			buffer[position] = c;
		}
		position++;
		
	if (position >= buffsize)
	{
		printf("BUFFER OVERFLOW \nINCREASING ALLOCATED MEMORY\n");
		buffsize += RL_BUFF_SIZE;
		buffer = realloc(buffer, buffsize);
		
		if(!buffer)
		{
			fprintf(stderr, "%stsh: ALLOCATION ERROR%s\n", RED, RESET);
			exit(EXIT_FAILURE);
			}
		}
	}
}



char **split_ln(char *line)
{
	int buffsize = TK_BUFF_SIZE, position = 0;
	char **tokens = malloc(buffsize*sizeof(char*));
	char *token;

	if(!tokens)
	{
		fprintf(stderr, "%stsh: ALLOCATION ERROR%s\n", RED, RESET);
		exit(EXIT_FAILURE);
	}
	token = strtok(line, TOK_DELIM);
	while(token != NULL)
	{
		tokens[position] = token;
		position++;

		if(position>=buffsize)
		{
			buffsize += TK_BUFF_SIZE;
			tokens = realloc(tokens, buffsize*sizeof(char*));

			if(!tokens)
			{
				fprintf(stderr, "%stsk: ALLOCATION ERROR%s\n", RED, RESET);
				exit(EXIT_FAILURE);
			}
		token = strtok(NULL, TOK_DELIM);
		}
	}

	tokens[position] = NULL;

	return tokens;
}





char **split_pipes(char *input)
{
	char *p = strtok(input, "|");
	char **s = malloc(1024*sizeof(char *));
	int i = 0;
	while(p != NULL)
	{
		s[i] = trimws(p);
		i++;
		p = strtok(NULL, "|");
	}
	s[i] = NULL;
	i=0;
	while(s[i] != NULL)
	{
		printf("%s\n", s[i]);
		i++;
	}
	return s;
}



char *trimws(char *str)
{
	char *end;
	while(isspace((unsigned char) * str)) str++;
	if(*str == 0)
		return str;
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char) *end)) end--;
	*(end+1) = 0;
	return str;
}



int args_length(char **args)
{
	int i = 0;

	while(args[i] != NULL)
	{
		i++;
	}
	return i;
}



int hist_ln_cnt()
{
	FILE *fp = fopen(get_hist_path(), "r");
	int c;
	int numOfLines = 1;
	do
	{
		c = getc(fp);
		if(c == '\n')
		{
			numOfLines++;
		}
	} while(c != EOF);
	return numOfLines;
}



int tsh_cd(char **args)
{
	if(args[1] == NULL)
	{
		fprintf(stderr, "%sTSH: ENTER PATH TO CHANGE TO%s\n" YELLOW, RESET);
	}
	else
	{
		if(chdir(args[1]) > 0)
		{
			perror("TSH");
		}
	}
	return 1;
}



int tsh_grep(char **args)
{
	FILE *fp = NULL;
	int flag = 0;
	char temp[512];
	int line_num = 1;
	if(args[0] != NULL && strcmp(args[0], "GREP") == 0)
	{
		if(args[1] != NULL && args[2] != NULL)
		{
			fp = fopen(args[2], "r");
			while((fgets(temp, 512, fp)) != NULL)
			{
				if(strstr(temp, args[1]))
				{
					printf("%d. %s", line_num, temp);
				}
				line_num++;
			}
			fclose(fp);
		}
		else
		{
			fprintf(stderr, RED "TSH: GREP MUST HAVE TWO PARAMETERS, " ITALICS "PATTERN" RESET RED " and " RED ITALICS "FILE" RESET "\n");
		}
	}
	if(flag == 0)
		printf("NO MATCH FOUND \n");
	return 1;
}



int tsh_help(char **args)
{
	if(args[0] != NULL && strcmp(args[0], "HELP") == 0)
	{
		fprintf(stderr,"\n------\n" BOLD "\nTSH " RESET "The default shell of TeberianOS, created on February 13, 2021.\n" "\nCOMMANDS:\n1. cd\n2. exit\n3. help\n4. touch\n5. cat" "\n\n------\n\n");
	}
	return 1;
}



int tsh_launch(char **args)
{
	int i = 0;
	if(args[0] == NULL)
	{
		return 1;
	}
	else if(strcmp(args[0], "HISTORY") != 0 && strcmp(args[0], "EXIT") != 0 && strcmp(args[0], "CLEAR") != 0)
	{
		hist_input(args, "");
	}
	for(i = 0; i<builtin_funcs_count(); i++)
	{
		if(strcmp(args[0], builtin_str[i]) == 0)
		{
			return (*builtin_funcs[i])(args);
		}
	}
	return tsh_exec(args);
}



int tsh_exec(char **args)
{
	pid_t cpid;
	int status;
	cpid = fork();

	if(cpid == 0)
	{
		if(execvp(args[0], args) < 0)
			printf("TSH: UNRECOGNIZED COMMAND: %s\n", args[0]);
		exit(EXIT_FAILURE);
	}
	else if(cpid < 0)
		printf(RED "ERROR FORKING" RESET "\n");
	else
	{
		waitpid(cpid, &status, WUNTRACED);
	}
	return 1;
}



int tsh_exit(char **args)
{
	return 0;
}



int tsh_hist()
{
	FILE *fp = fopen(get_hist_path(), "r");
	int ch, c, line_num = 1;
	char line[128];
	char prev_comm[128];
	char **args=NULL;
	if(!fp)
		fprintf(stderr, RED "tsh: FILE NOT FOUND" RESET "\n");
	else
	{
		putchar('\n');
		while((c = getc(fp)) != EOF)
		{
			putchar(c);
		}
	}
	printf( "\n" INVERT " <0>: QUIT  <#line>: EXEC CMD <-1>: CLR HIST" RESET "\n\n:  ");
	scanf("%d", &ch);
	getchar();
	fseek(fp, 0, SEEK_SET);
	if(isdigit(ch) != 0)
	{
		printf("ENTER A NUMERICAL CHOICE\n");
	}
	else if (ch == 0)
	{
		fclose(fp);
		return 1;
	}
	else if(ch == -1)
	{
		fclose(fp);
		fp = fopen(get_hist_path(), "w");
		fclose(fp);
		return tsh_exec(clr);
	}

	else
	{
		while((fgets(line, 128, fp)) != NULL)
		{
			if(line_num == ch)
			{
				strcpy(prev_comm, &line[3]);
				int p = 0, flag = 0;
				fclose(fp);
				while(prev_comm[p] != '\0')
				{
					if(prev_comm[p] == '|')
					{
						flag = 1;
						break;
					}
						p++;
					}
					if(!flag)
					{
						args = split_ln(prev_comm);
						return tsh_launch(args);
					}
					else
					{
						args = split_pipes(prev_comm);
						return tsh_pipe(args);
					}
				}
				else
					line_num++;
			}
	}
	return 1;
}




int tsh_pipe(char **args)
{
	int tempin=dup(0);
	int tempout=dup(1);
	int j=0, i=0, flag=0;
	int fdin = 0, fdout;


	for(j = 0; j<args_length(args); j++)
	{
		if(strcmp(args[j], "<") == 0)
		{
			fdin=open(args[j+1], O_RDONLY);
			flag += 2;
		}
	}

	if(!fdin)
		fdin=dup(tempin);
	int pid;
	for(i = 0; i<args_length(args)-flag; i++)
	{
		char **rargs = split_ln(args[i]);
		dup2(fdin, 0);
		close(fdin);
		if(i == args_length(args)-3 && strcmp(args[i+1], ">") == 0)
		{
			if((fdout = open(args[i+1], O_WRONLY)))
				i++;
		}
		else if(i == args_length(args)-flag-1)
			fdout = dup(tempout);
		else
		{
			int fd[2];
			pipe(fd);
			fdout = fd[1];
			fdin = fd[0];
		}

		dup2(fdout, 1);
		close(fdout);

		pid = fork();
		if(pid == 0)
		{
			execvp(rargs[0], rargs);
			perror("ERROR FORKING\n");
			exit(EXIT_FAILURE);
		}

		wait(NULL);
	}

	dup2(tempin, 0);
	dup2(tempout, 1);
	close(tempin);
	close(tempout);

	return 1;
}
		

void get_dir(char *state)
{
	char cwd[1024];
	if(getcwd(cwd, sizeof(cwd)) != NULL)
	{
		if(strcmp(state, "LOOP") == 0)
			printf(RED "[ " RESET CYAN "%s" RESET RED " ] " RESET, cwd);
	else if(strcmp(state, "pwd") == 0)
		printf("%s\n", cwd);
	}
	else
	{
		printf("%sgetcwd() error%s", RED, RESET);
	}
}





void hist_input(char **args, char *d)
{
	FILE *hist_file = fopen(get_hist_path(), "a+");
	int j = 0;
	fprintf(hist_file, "%d. ", hist_ln_cnt());
	while(args[j] != NULL)
	{
		if(j > 0)
			fputs(d, hist_file);
		fputs(args[j], hist_file);
		j++;
	}
	fputs("\n", hist_file);
	fclose(hist_file);
}

void pipe_hist_input(char *line)
{
	FILE *hist_file = fopen(get_hist_path(), "a+");
	fprintf(hist_file, "%d. %s\n", hist_ln_cnt(), line);
	fclose(hist_file);
}


void loop()
{
	char *line;
	char **args;
	int status=1, i = 0, flag = 0;

	do {
		get_dir("loop");
		printf(YELLOW "TEB-> " RESET);
		line = read_ln();
		flag = 0;
		i = 0;
		while(line[i] != '\0')
		{
			if(line[i] == '|')
			{
				flag = 1;
				break;
			}
			i++;
		}
		if(flag)
		{
			pipe_hist_input(line);
			args = split_pipes(line);
			status = tsh_pipe(args);
		}
		else
		{
			args = split_ln(line);
			status = tsh_launch(args);
		}
		free(line);
		free(args);
	} while(status);
}



void print_tkn(char **tokens)
{
	int i = 0;
	while(tokens[i] != NULL)
	{
		printf("%s\n", tokens[i]);
		i++;
	}
}



void signalHandler()
{
	signal(SIGINT, signalHandler);
	getchar();
}






int main(int argc, char **argv)
{
	loop();
	return EXIT_SUCCESS;
}
