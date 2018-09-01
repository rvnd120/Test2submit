1. Direction of Compilation:

Compile and run the code as root user. Because I am reading and writing files into /var/run directory which is owned by root and only writable by root.

2. Command for Compilation: 

If you run the code directly as root user then command for compilation is,

 cc -Wall -o daemon_process daemon_process.c  


3. Commands to run the code:

a. ./daemon_process start    - To start the daemon
b. ./daemon_process stop     - To stop the daemon  
c. ./daemon_process restart  - To restart the daemon
d. ./daemon_process status   - To know the status of the daemon 

4. Assumptions made:
 
a. Configuration file does not end with new line character.
b. only 3 lines in the config file.
c. In case of failing connection time stamp to the log file can take bit longer time than usual. Since connection time out time by connect() is bit longer.

5.  Steps to create daemon process:

a. Create a normal process.
b. Fork() off the above parent process to create child process.
c. Terminate the parent process.
d. Call setsid() function to run the above orphaned child process in new session.
e. Fork off for the second time and it's parent (i.e above process)
f. Now daemon is created without a terminal.

In my code I save PID of daemon in daemon_process.pid to check for daemon process existence to start, stop, restart or to know the status of my daemon_process code. Once the daemon_process code is started it reads the configuration file and pings the IP address specified in the file on specified regular intervals. It logs the result into a text file with timestamp and in case of failure of connection it executes specified shell command in the config file. I used this command echo "ALERT" >> /var/log/connectivity.log  
in case of failure of connection.
