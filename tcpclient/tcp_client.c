#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	// Validate number of arguments
	if (argc != 3) {
		fprintf(stderr, "USAGE: tcpclient <server IP> <server port>\n");
		exit(1);
	}

	// Validate IPv4 address
	unsigned char netaddr[sizeof(struct in_addr)];
	int rval;
	if ((rval = inet_pton(AF_INET, argv[1], netaddr)) <= 0) {
		if (rval == 0)
			fprintf(stderr, "IP address not in presentation format\n");
		else
			perror("inet_pton");
		exit(1);
	}

	// Check length of port. If greater than 5, not a valid port
	if (strlen(argv[2]) > 5) {
		fprintf(stderr, "Invalid port\n");
		exit(1);
	}

	// Validate port by converting to string to base 10 long and checking
	// that it is within bounds of possible port ranges
	char *endptr;
	
	long port = strtol(argv[2], &endptr, 10);
	if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN))
			|| (errno != 0 && port == 0)) {
		perror("strtol");
		exit(1);
	}

	if (endptr == argv[2]) {
		fprintf(stderr, "No digits found in port number\n");
		exit(1);
	}

	// Check if input contained extra alphabetic characters
	if (*endptr != '\0') {
		fprintf(stderr, "Invalid port\n");
		exit(1);
	}
	
	// Successfully converted string to long, check if within range
	if (port < 1 || port > 65535) {
		fprintf(stderr, "Port is out of range\n");
		exit(1);
	}

	// Create IPv4 TCP socket and get socket file descriptor
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		// Error occurred
		perror("socket");
		exit(1);
	}

	// Socket file descriptor returned successfully
	// Fill address info structure
	struct sockaddr_in servinfo;
	servinfo.sin_family = AF_INET;
	servinfo.sin_port = htons(port); // Convert port to network byte order	
	servinfo.sin_addr.s_addr = *netaddr;
	
	// Connect to server
	if ((connect(sockfd, (struct sockaddr *) &servinfo, sizeof(servinfo)) == -1)) {
		perror("connect");
		exit(1);
	}

	// Send message to the server
	char mymsg[] = "This is super cool!";
	send(sockfd, mymsg, sizeof(mymsg), 0);
	
	// Receive data from server
	char rcvd[128];
	recv(sockfd, rcvd, sizeof(rcvd), 0);

	// Print received message
	printf("Server says: %s", rcvd);	

	return 0;
}
