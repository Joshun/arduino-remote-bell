/* Arduino server via pipes, for use with php
 * Joshun 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>

#define FIFO_PATH "/tmp/my_fifo.in"
#define FIFO_FEEDBACK_PATH "/tmp/my_fifo.out"

#define BUFFER_SIZE PIPE_BUF
#define ARDUINO_BUFFER_SIZE PIPE_BUF

#define DEVICE_PATH "/dev/ttyACM0"
#define DEVICE_BAUD B115200
#define END_CHAR '\n'
#define EOT 0x4

int arduino_init(struct termios *old_config, struct termios *new_config);

int main()
{
	/* arduino stuff */
	struct termios arduino_config;
	struct termios backup_config;
	
	char arduino_buffer[ARDUINO_BUFFER_SIZE + 1];
	int arduino_fd = arduino_init(&backup_config, &arduino_config);
		
	/* server stuff */
	int pipe_fd;
	int pipe_out_fd;
	int read_once;
	int open_mode = O_RDONLY;
	int open_out_mode = O_WRONLY;
	char buffer[BUFFER_SIZE + 1];
	char name_buffer[BUFFER_SIZE + 1];
	int bytes_read = 0;
	
		/* if fifo does not exist / can't be accessed, create it */
	if( access(FIFO_PATH, F_OK) == -1 ) {
		read_once = mkfifo(FIFO_PATH, 0777);
		if( read_once != 0 ) {
			/* abort on failure to create fifo */
			fprintf(stderr, "Could not create FIFO at %s\n", FIFO_PATH);
			exit(EXIT_FAILURE);
		}
	}
	
	if( access(FIFO_FEEDBACK_PATH, F_OK) == -1 ) {
		read_once = mkfifo(FIFO_FEEDBACK_PATH, 0777);
		if( read_once != 0 ) {
			/* abort on failure to create fifo */
			fprintf(stderr, "Could not create FIFO at %s\n", FIFO_PATH);
			exit(EXIT_FAILURE);
		}
	}
	
	memset(buffer, '\0', sizeof(buffer));
	memset(name_buffer, '\0', sizeof(name_buffer));
	memset(arduino_buffer, '\0', sizeof(arduino_buffer));
	
	while(1) {
		printf("Process %d opening FIFO O_RDONLY\n", getpid());
		pipe_fd = open(FIFO_PATH, open_mode);
		printf("Process %d opened fifo on fd %d\n", getpid(), pipe_fd);
		pipe_out_fd = open(FIFO_FEEDBACK_PATH, open_out_mode);
		
		if( pipe_fd != -1 ) {
			do {
				read_once = read(pipe_fd, buffer, BUFFER_SIZE);
				if(read_once == 0)
					break;
				bytes_read += read_once;

			/* safety check preventing overflow */
			if((bytes_read + 10) > BUFFER_SIZE)
			{
				printf("Warning: large amount of data, cutting...\n");
				break;
			}
			
			} while ( read_once > 0 );
			
			buffer[bytes_read] = '\n';
			buffer[bytes_read + 1] = '\0';
			printf("Dump of received data: %s\n", buffer);
			
			int i, j, end_index;
			for(i=0; i<bytes_read; i++)
			{
				if(buffer[i] == EOT)
				{
					end_index = i;
					printf("EOT found\n");
					break;
				}
			}
			
			j = end_index + 1;
			
			for(i=0; i<bytes_read; i++)
			{
				if( buffer[j] == '\0'  || buffer[j] == EOT)
					break;
				
				name_buffer[i] = buffer[j];
				j++;
			}
			
			printf("Sent from %s\n", name_buffer);
			j++;
			for(i=0; i<bytes_read; i++)
			{
				if( buffer[j] == '\0' )
					break;
				name_buffer[i] = buffer[j];
				j++;
			}
			name_buffer[i] = '\0';
			
			printf("Message: %s\n", name_buffer);
			
			buffer[end_index] = '\0';
			
			printf("Closing FIFO...\n");
			close(pipe_fd);
			
			
			
			write(arduino_fd, buffer, strlen(buffer));
			
			int nread, result;
			char ch = 0;
			for(nread = 0; ch != END_CHAR; nread++)
			{
				result = read(arduino_fd, &ch, 1);
				arduino_buffer[nread] = ch;
				printf("Read %c (%d)\n", ch, ch);
				if( result < 0 )
				{
					printf("Warning: read error\n");
					break;
				}
					
				if( (nread + 2) >= ARDUINO_BUFFER_SIZE )
				{
					printf("Warning: large buffer size\n");
					break;
				}
						
			}

			arduino_buffer[nread] = '\0';
			
			
			//printf("Response: %d\n", arduino_buffer[0]);
			printf("Response (%d): %s\n", nread, arduino_buffer);
			write(pipe_out_fd, arduino_buffer, strlen(arduino_buffer));
			memset(arduino_buffer, '\0', nread);
			
			memset(buffer, '\0', bytes_read);
			memset(name_buffer, '\0', sizeof(name_buffer));
			close(pipe_out_fd);
		}
		else {
			exit(EXIT_FAILURE);
		}
		
	}
}

int arduino_init(struct termios *old_config, struct termios *new_config)
{
	int fd = open(DEVICE_PATH, O_RDWR | O_NOCTTY);
//	int fd = open(DEVICE_PATH, O_RDWR);

	
	if(fd == -1)
	{
		printf("Arduino not connected. Exiting...\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Arduino fd opened as %i\n", fd);
	tcgetattr(fd, old_config);
	new_config = old_config;
	cfsetispeed(new_config, DEVICE_BAUD);
	cfsetospeed(new_config, DEVICE_BAUD);
	
	new_config->c_cflag &= ~PARENB;
	new_config->c_cflag &= ~CSTOPB;
	new_config->c_cflag &= ~CSIZE;
	new_config->c_cflag |= CS8;
	/* Canonical mode */
	//new_config->c_lflag |= ICANON;
	new_config->c_lflag |= ICANON;
	/* commit the serial port settings */
	tcsetattr(fd, TCSANOW, new_config);
	return fd;
}
