/*Author: Aravind Baskaran */
/*Language: C */
/*platform: Ubuntu 16.04.4  LTS */
/*Tool used: Vim editor */
/*Version of Vim: Vim - Vi improved 7.4 */
/*Compiler used: gcc compiler */
/*Compiler version: gcc 5.4.0 */
  

/*File path definitions*/
#define FILEPATH "/home/aravind/config6.txt"
#define FPIDPATH "/var/run/daemon_process.pid"
#define RESULTFPATH "/home/aravind/result.txt"

#define DELIM "."          /* To parse through the IP address string to check whether it's in valid IPv4 format*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include<sys/socket.h>
#include<arpa/inet.h>     /* To inet_addr() fnction to convert IPv4 internet address to binary data in network byte order */ 
#include<netinet/in.h>

/*File pointer for log file of connection*/
FILE *fresult;

/*PID file file pointer*/
FILE *fpid;                                                     

const char *pidfile=FPIDPATH;

char command[1024];						 	
int  interval=0;
char ipaddress[1024];

char killid[1024];

/*Start Status variable to know daemon is already running or not*/
int start_status=1; 

/*Restart Status variable to know daemon is already running to restart or not*/                                           
int restart_status=1;                                          

/*Function to check IP address contains valid digits*/
int valid_digit(char *ip_str)                                 
{
	while (*ip_str) {
		if (*ip_str >= '0' && *ip_str <= '9')
		{
			++ip_str;
		}		
		else
		{
			return 0;
		}
	}
	return 1;
}

/*Function to check IP address is in proper IPv4 fromat or not*/
int is_valid_ip(char *ip_str)                      
{
	int  num, dots = 0;                                       
	char *ptr;

	if (ip_str == NULL)
	{
		return 0;
	}
	ptr = strtok(ip_str, DELIM);


	if (ptr == NULL)
	{       
		return 0;
	}
	while (ptr)
	{

		/* after parsing string, it must contain only digits */
		if (!valid_digit(ptr))
		{			
			return 0;
		}
		num = atoi(ptr);

		/* check for valid IP */
		if (num >= 0 && num <= 255) 
		{       
			/* parse remaining string */
			ptr = strtok(NULL, DELIM);
			if (ptr != NULL)
			{
				++dots;
			}		
		} else
		{
			return 0;
		}
	}

	/* valid IP string must contain 3 dots */
	if (dots != 3)
	{

		return 0;
	}
	return 1;
}

/*Function to check interval to ping is a valid digit or not*/
int interval_check(char *value)
{  
	/*isdigit returns O on success*/
	while(*value)
	{
		if(isdigit(*value++) == 0)
		{
			return 0;
		}
	}
	return 1;
}

/*Function to remove new line character from strings read from config file*/

void remove_newline(char string[])
{
	int i=0;
	while(string[i] != '\0')
	{
		if(string[i] == '\n')
		{
			string[i]='\0';
			break;
		}

		i++;
	}
}  

/*Daemon Function*/
void newdaemon()
{
	pid_t pid;

	/* Fork off the parent process */
	pid = fork();

	/* To check for error */
	if (pid < 0)
	{
		exit(EXIT_FAILURE);
	}
	/* Let the parent terminate */
	if (pid > 0)
	{
		exit(EXIT_SUCCESS);
	}
	/* On success te child process becomes session leader */
	if (setsid() < 0)
	{
		exit(EXIT_FAILURE);
	}
	/* Fork off for the second time*/
	pid = fork();

	/* To check for error */
	if (pid < 0)
	{
		exit(EXIT_FAILURE);
	}
	/*Let the parent terminate */
	if (pid > 0)
	{
		exit(EXIT_SUCCESS);
	}
	/*Change the file mode mask*/
	umask(0);
}

/*Function to ping the given IP address*/
void ping(int interval, char * ipaddress, char * command)
{	

	time_t curtime;

	/*loop continuously to ping the passed IP address on specified interval*/
	while(1)
	{
		/*Varible to store the return value of connect() function*/
		int check;
		time(&curtime);
		
		/* Socket file descriptor variable to connect to server of given IP address*/
		int socket_desc;
		struct sockaddr_in serverAddr;
		socklen_t addr_size ;

		/* Socket() function to create endpoint of communication and to get file descriptor of that endpoint*/
		socket_desc = socket(AF_INET , SOCK_STREAM , 0);

		if (socket_desc == -1)
		{
			printf("Could not create socket");
		}

		/*inet_addr() fnction to convert IPv4 internet address to binary data in network byte order */
		serverAddr.sin_addr.s_addr = inet_addr(ipaddress);


		serverAddr.sin_family = AF_INET;


		/*htons() function to convert unsigned short integer from host byte order to network byte order i.e check for endianess*/
		serverAddr.sin_port = htons(53);


		addr_size=sizeof(serverAddr);
		

		/*connect() function to make a connection on a socket to ping the server of given IP address*/
		check=connect(socket_desc ,(struct sockaddr *) &serverAddr , addr_size);		
	
		/*To check Connection is established or not*/
		if(check == 0)
		{

			/*Creating a file if not existing and appending the result success of connection  with timestamp*/
			fresult=fopen(RESULTFPATH,"a");

			fprintf(fresult,"Success on connecting to ip address %s on %s",ipaddress,ctime(&curtime));
			fclose(fresult);

			/*To properly shutdown the exixting connection and closing the socket file descriptor*/
			shutdown(socket_desc,2);

		}

		else if(check < 0)
		{
			/*Creating a file if not existing and appending the result failure of connection  with timestamp*/
			fresult=fopen(RESULTFPATH,"a");

			fprintf(fresult,"Failure on connecting to ip address %s on %s",ipaddress,ctime(&curtime));
			fclose(fresult);

			/*To properly shutdown the exixting connection and closing the socket file descriptor*/
			shutdown(socket_desc,2);

			/*system() function to execute the given shell command  in the config file in case of failure of connecting to server*/
			system(command);

		}
	
		

	sleep(interval);		

	}
}

/* Function to start daemon*/
void startfn(void)
{	
	/* To check for the existence of pid file with process ID*/
	fpid=fopen(pidfile,"rb");

	/* To check daemon already running or not with PID file existence if running returns 0*/
	if(fpid == 0)
	{
		printf("starting the daemon\n");

		/*Daemon Started*/
		newdaemon();

		/*Create PID file and write process ID for further operations like stop, restart, status check*/
		fpid=fopen(pidfile,"w");
		fprintf(fpid,"%d",getpid());
		fclose(fpid);
	}
	else
	{
		fclose(fpid);
		start_status = 0;


	}
}

/* Function to stop daemon*/
void stopfn(void)
{	
	/* To check for the existence of pid file with process ID*/
	fpid=fopen(pidfile,"r");

	/* To check daemon already in stop state or not with PID file existence*/
	if(fpid == 0)
	{

		printf("Already daemon is in stop state\n");

	}
	else
	{
		/*Reads and copy the PID of runnign daemon to kill it*/		
		fscanf(fpid,"%s",killid);

		fclose(fpid);
		kill(atoi(killid),SIGTERM);

		/*Delete the exixting PID file with PID to facilitate next start of daemon*/
		unlink(pidfile);

		printf("Daemon is killed\n");	

	}

}

/*Function to check status of daemon*/
void statusfn(void)
{	
	/* To check for the existence of pid file with process ID*/
	fpid=fopen(pidfile,"r");

	/* To know daemon is in running or stop state with PID from PID file if exists*/
	if(fpid != 0)
	{
		fclose(fpid);
		printf("Daemon is in running state\n");

	}
	else
	{

		printf("Daemon is in stop state\n");

	}

}

/*Function to restart the daemon*/
void restartfn(void)
{	
	/* To check for the existence of pid file with process ID*/

	fpid=fopen(pidfile,"r");

	/* To know daemon is in running or stop state with PID from PID file if exists*/
	if(fpid == 0)
	{

		printf("Daemon is in stop state\n");
		restart_status=0;
	}
	else
	{	/*Read PID file for process ID to kill the already running daemon by sigterm*/
		fscanf(fpid,"%s",killid);
		fclose(fpid);

		/*Daemon Killed*/
		kill(atoi(killid),SIGTERM);
		printf("Shutting down currently running daemon\n");

		/*Delete the exixting log file &  PID file with PID to facilitate next start of daemon*/
		unlink(pidfile);
		unlink(RESULTFPATH);

		/*Daemon Restart*/
		printf("Restarting the daemon\n");
		newdaemon();

		/*Create a new PID file again and write process ID for further operations like stop, restart, status check*/
		fpid=fopen(pidfile,"w");
		fprintf(fpid,"%d",getpid());
		fclose(fpid);

	}
}

/* Function to read configuration file to fetch the neccessary details for pinging*/
int configuration(void)
{	
	FILE *fconfig;
	char *configfile = FILEPATH;
	int interval_success=0;
	int ip_success=0;

	/* Variable to note the number of lines read from file*/
	int readcount=0;
	/*variable to note the format error of necessary details in file*/
	int errorformat=0;

	char line[1024];
	char copy[1024];
	fconfig=fopen(configfile,"rb");
	if(fconfig != 0)
	{       
		/*fgets to read line by line of config file*/
		while((fgets(line,1024,fconfig)!=0))
		{       
			/*To ensure we fetched the neccesary string without new line character*/
			remove_newline(line);
			strcpy(copy,line);


			/*To Check for the validity of IP address in file*/
			if((ip_success = is_valid_ip(copy))==1)
			{
				strcpy(ipaddress,line);

			}

			/*To check for the validity of interval to ping in the file*/
			else  if((interval_success = interval_check(line))==1 )
			{       
				/*To convert string to a digit*/
				interval=atoi(line);

			}

			/*To copy the command to execute in case of pinging failure*/
			else
			{	/*To note the format errors of the IP address and interval in the file*/
				errorformat++;

				strcpy(command,line);

			}

			/*To check number of lines in file is more than 3 or less than 3*/
			readcount++;
		}
		fclose(fconfig);	
	}
	/* To check config file is in proper format or not. Returns 0 if file not in proper format */
	if(readcount >= 4 || errorformat >1 || readcount <=2)
	{	
		return 0;
	}
	return 1;
}

/* Main function to receive daemon function  arguments and initiate ping function */
int main(int argc, char *argv[])
{ 	
	/*Variable to receive from configuration function to check format errors of config file*/
	int  formaterror;

	if((formaterror=configuration())==0)

	{     	
		printf("\nConfiguration file format error\n\n");
		printf("Due to one of the reasons below:\n\n");
		printf("1. Too many details which is not required\n\n");
		printf("2. Too few details\n\n");
		printf("3. Ended with new line not right format\n\n");
		printf("4. Necessary details are not in right format\n\n");	
		return 0;
	}

	/*Strcmp function used to know what operation to be perfomed. strcmp returns 0 if strings are same*/
	/*Varibles start,restart,stop,status to store return value of strcmp function used below*/

	int start,restart,stop,status;	
	start=strcmp("start",argv[1]);
	restart=strcmp("restart",argv[1]);
	stop=strcmp("stop",argv[1]);
	status=strcmp("status",argv[1]);


	/*To check for whether proper number of arguments passed or not*/
	if(argc == 2)

	{
		/*To check for the arguments passed to perform the necessary daemon function like start, stop, restart, status*/
		if(start == 0)

		{		/*Start function call to check daemon is already running or not. If not, then to start the dameon*/
			startfn();
			if(start_status == 0)

			{
				printf("Daemon is already running\n");
				return 0;	
			}
		}	

		else if(stop == 0)

		{       
			/*Stop function call to check daemon is already in stop state or not. If not, then to stop the dameon*/
			stopfn();
			return 0;

		}

		else if(status == 0)

		{	
			/*Status function call to check daemon is in running start or stop state */
			statusfn();
			return 0;
		}


		else if(restart == 0)

		{		

			/*Restart function call to check daemon is already running. If running, then kill the running daemon and restart*/
			restartfn();
			if(restart_status == 0)

			{
				return 0;

			}			


		}	

		/*To check whether proper arguments with proper format passed to main function*/
		else if((start != 0) && (stop != 0) && (restart != 0) && (status != 0))

		{

			printf("Enter Proper arguments\n");
			return 0;

		}



	}

	/*To fail the daemon start if proper number of arguments are not passed with a prompt*/
	else 
	{
		printf("Too few or too many arguments so daemon wont start\n");
		return 0;
	}


	while(1)
	{
		/*Function call to ping the IP address given in the config file*/
		ping(interval,ipaddress,command);

	}	        
	return 0;
}



