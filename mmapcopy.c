       #include <sys/mman.h>
       #include <sys/stat.h>
       #include <fcntl.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <sys/wait.h>

       int main(int argc, char *argv[])
       {
           char *buffer;
           int fd;
           struct stat info;
           size_t length;
           ssize_t s;
	       pid_t pid; 
           int status;

            // check if the file name passed as argument or exit if error
    
           // open the file here or exit if error
           
           if (fstat(fd, &info) == -1) {          
               printf("%s empty", argv[1]);
               exit(0);
           }
	       if ((pid = fork()) ==  0) { 

		        printf("Child process\n");           	
                // Child process copy the arbitrary-sized disk
           	    // and check if an error occurs 
           	    // child process write out the file content
        
            	printf("CP: done\n");
        	    exit(1);
            }
            printf("Parent process\n");  
	        // parent wait
	        printf("PP: done\n");
            exit(1);
        }