#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

struct stat *buf;
FILE *currentFile;
pid_t pid;
unsigned int countProcess = 1, maxCountProcess;
char *basename;

int findChar(char * filePath)
{
	while (countProcess >= maxCountProcess)
	{
		wait(NULL);
		countProcess--;	
	}
	countProcess++;
	pid = fork();
	if (pid < 0) 
	{
		printf("Can not creat new process.\n");
		return errno;
		countProcess--;
	}
	if (pid > 0)
		return 0;	
	
	unsigned long long countByte = 0;
	unsigned long long frequency[257];
	int c;
	for(int i=0;i<256;i++)
		frequency[i] = 0;
		
	currentFile = fopen(filePath,"rb");
	if (currentFile == NULL) 
	{
		fprintf(stderr,"fopen : %s \n",strerror(errno));
		exit(0);
	}
	while ((c=getc(currentFile))!=EOF)
	{
		frequency[c]++;
		countByte++;
	}
	char output[5000];
	fclose(currentFile);
	output[0]=0;
	char intc[32];
	sprintf(output,"\n %d : %s : %llu :", getpid(), filePath, countByte);
	
	for (int i=0;i<256;i++)
	{
		if(frequency[i]>0)
		{
			sprintf(intc,"  #%d-%llu",i,frequency[i]);
			strcat(output,intc);
		}	
	}
	
	strcat(output,"\n");
	//fflush(stdout);
	//fprintf(stdout,"%s", output);
	sleep(1);
	puts(output);
	//fflush(stdout);
	exit(0);
}	

int viewDir(char *folder)
{
    struct dirent *currentDir;
    char* filePath;
    DIR* dirPtr;

    if((dirPtr = opendir(folder)) == NULL)
    {
        fprintf(stderr, "%s : opendir : %s : %s\n", basename, strerror(errno), folder);
		return errno;
    }
    int error;
    while (1)
    {
        error=errno; 
        currentDir = readdir(dirPtr);
        if(currentDir == NULL && error!=errno)
        {
            fprintf(stderr, "readdir : %s : %s\n",  strerror(errno), folder);
		    return errno;
        }
        if (currentDir == NULL)
        {
            break;        
        }
        filePath = (char*)malloc(sizeof(char)*(strlen(folder)+1));
        strcpy(filePath,folder);

        if (currentDir->d_type == DT_LNK)
		{
			continue;
		}
        
        if (currentDir->d_type == DT_DIR)
        {
            if ((currentDir->d_type == DT_DIR) && (strcmp(currentDir->d_name, ".") != 0) && (strcmp(currentDir->d_name, "..") != 0))
			{                     
                filePath  = (char*)realloc(filePath,sizeof(char)*(strlen(filePath)+2+strlen(currentDir->d_name)));       
                strcat(filePath,"/");
                strcat(filePath,currentDir->d_name); 
                viewDir(filePath);  
            }         
        }   
        else
        {
            filePath  = (char *)realloc(filePath,sizeof(char)*(strlen(filePath)+3+strlen(currentDir->d_name)));
            strcat(filePath,"/");
            strcat(filePath,currentDir->d_name);
            if (stat(filePath, buf) != 0)
			{
				fprintf(stderr, "stat : %s : %s\n", strerror(errno), filePath);
				continue;
			}
            findChar(filePath);  
            if (pid==0)
				exit(0);
        }    
    }
    free(filePath);
    if(closedir(dirPtr))
    {
        fprintf(stderr, "closedir : %s : %s\n",  strerror(errno), folder);
		return errno;
    }
    return 0;
}

char *realname(char *arg)
{
	int len = strlen(arg);
	int i = len-1;

	while(arg[i] != '/')
		i--;
	
	int lentemp = len - i;
	char *tempstr = malloc(sizeof(char)*(lentemp));
	int j,k = 0;
	
	i++;
	
	for(j = i; j < len; j++)
		tempstr[k++]=arg[j];
	tempstr[k]=0;

	return tempstr;	
}

int main(int argc, char *argv[],char *envp[])
{
	char *dirname;
	buf = (struct stat*) malloc (sizeof(struct stat));
	basename = realname(argv[0]);
	if (argc < 3) 
	{
		fprintf(stderr,"%s : Not enough parameters!\n", basename);
		return 1;
	}
	maxCountProcess = atoi(argv[2]);
	dirname = argv[1];
	viewDir(dirname);
	while (countProcess>1)
	{
		wait(0);
		countProcess--;
	}
	return 0;
}	
