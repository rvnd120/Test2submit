1. Direction of Compilation:

Compile and run the code as root user. Because I am reading and writing files into /var/run directory which is owned by root and only writable by root.

2. Command for Compilation: 

 sudo cc -Wall -o daemon_process daemon_process.c

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


