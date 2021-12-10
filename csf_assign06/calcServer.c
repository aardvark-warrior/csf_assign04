/** CSF Assignment 6 Server 
 * 
 * Arthur Wang - awang91
 * Mason Albert - malber20
 * 
 */

#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"
#define LINEBUF_SIZE 1024

int chat_with_client(struct Calc *calc, int client_fd);

/*
 * Data structure representing a client connection.
 */
struct ConnInfo {
	int client_fd;
	struct Calc* calculator;
};

void *worker(void *arg) {
	struct ConnInfo *info = arg;

	/*
	 * set thread as detached, so its resources are automatically
	 * reclaimed when it finishes
	 */
	pthread_detach(pthread_self());

	/* handle client request */
	chat_with_client(info->calculator, info->client_fd);
	close(info->client_fd);
	free(info);

	return NULL;
}

int main(int argc, char **argv) {	
	if (argc != 2) { 
		fprintf(stderr, "Usage: ./calcServer <port>\n"); 
		return 1;
	}

	char* end_ptr = argv[1];
	
	while (*end_ptr != '\0') { end_ptr++; }

	unsigned long tcp_port = strtoul(argv[1], &end_ptr, 0);

	if (tcp_port == 0 && !((end_ptr == argv[1] + 1) && (*end_ptr == 0))) { // if 0 due to strtoul fail
		fprintf(stderr, "Invalid format for port number\n");
		return 2;
	}
	else if (tcp_port < 1024) {  // if invalid port number
		fprintf(stderr, "Invalid port number\n");
		return 3;
	}

	// Open server socket
	int server_fd = open_listenfd(argv[1]);
	if (server_fd < 0) { 
		fprintf(stderr, "Couldn't open server socket\n"); 
		return 4;
	}

	struct Calc* calculator = calc_create(); 
	
	int keep_going = 1;
	while (keep_going) {
		int client_fd = Accept(server_fd, NULL, NULL);
		if (client_fd < 0) {
			fprintf(stderr, "Error accepting client connection\n");
			return 5;
		}

		/* create ConnInfo object */
		struct ConnInfo *info = malloc(sizeof(struct ConnInfo));
		info->client_fd = client_fd;
		info->calculator = calculator;

		/* start new thread to handle client connection */
		pthread_t thr_id;
		if (pthread_create(&thr_id, NULL, worker, info) != 0) {
			fprintf(stderr, "Pthread_create failed\n"); 
			// return 6;
		}
	}

	calc_destroy(calculator);
	close(server_fd); // close server socket

	return 0;
}

int chat_with_client(struct Calc *calc, int client_fd) {
	rio_t in;
	char linebuf[LINEBUF_SIZE];

	/* wrap standard input (which is file descriptor 0) */
	rio_readinitb(&in, client_fd);

	/*
	 * Read lines of input, evaluate them as calculator expressions,
	 * and (if evaluation was successful) print the result of each
	 * expression.  Quit when "quit" command is received.
	 */
	int done = 0;
	while (!done) {
		ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
		if (n <= 0) {
			/* error or end of input */
			done = 1;
		} else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
			/* quit command */
			done = 1;
		} else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
			/* quit command */
			return 0;
		} else {
			/* process input line */
			int result;
			if (calc_eval(calc, linebuf, &result) == 0) {
				/* expression couldn't be evaluated */
				rio_writen(client_fd, "Error\n", 6);
			} else {
				/* output result */
				int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
				if (len < LINEBUF_SIZE) {
					rio_writen(client_fd, linebuf, len);
				}
			}
		}
	}
	return 1;
}
