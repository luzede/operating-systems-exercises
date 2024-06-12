#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    if (argc > 6) {
        printf("Usage: './ask4 [--host HOST] [--port PORT] [--debug]'\n");
        exit(0);
    }


    int debug = 0;
    char HOST[200] = {0}; // Initialize the array with null-termination characters
    int PORT = -1;

    for (int i = 1; i < argc; i++) {
        // Checking for the debug flag
        if (strcmp(argv[i], "--debug") == 0 && debug != 1) {
            debug = 1;
            continue;
        }
        else if (strcmp(argv[i], "--debug") == 0 && debug == 1) {
            printf("Usage: './ask4 [--host HOST] [--port PORT] [--debug]'\n");
            printf("You entered '--debug' twice\n");
            exit(0);
        }

        // Checking for the host flag
        if (strcmp(argv[i], "--host") == 0 && strlen(HOST) == 0) {
            if (i + 1 < argc) {
                snprintf(HOST, 200, "%s", argv[i + 1]);
                i++;
                continue;
            }
            else {
                printf("Usage: './ask4 [--host HOST] [--port PORT] [--debug]'\n");
                printf("You entered '--host' but did not provide a HOST\n");
                exit(0);
            }
        }
        else if (strcmp(argv[i], "--host") == 0 && strlen(HOST) != 0) {
            printf("Usage: './ask4 [--host HOST] [--port PORT] [--debug]'\n");
            printf("You entered '--host' twice\n");
            exit(0);
        }

        // Checking for the port flag
        if (strcmp(argv[i], "--port") == 0 && PORT == -1) {
            if (i + 1 < argc) {
                char *pEnd;
                long t = strtol(argv[i + 1], &pEnd, 10);
                // If 'pEnd' does not point to the null character
                // then the string is not in the correct number format
                if (*pEnd != '\0') {
                    printf("Usage: './ask4 [--host HOST] [--port PORT] [--debug]'\n");
                    printf("You entered '--port' but did not provide a number\n");
                    exit(0);
                }

                // If the value of port number is outside the range of 0 to 65535
                // then it is not a valid port number
                if (t > 65535 || t < 0) {
                    printf("Usage: './ask4 [--host HOST] [--port PORT] [--debug]'\n");
                    printf("The value of 'PORT' is out of range\n");
                    exit(0);
                }

                // Converting 'long' to 'int' and assigning it to 'PORT'
                PORT = (int) t;
                i++;
                continue;
            }
            else {
                printf("Usage: './ask4 [--host HOST] [--port PORT] [--debug]'\n");
                printf("You entered '--port' but did not provide a PORT\n");
                exit(0);
            }
        }
        else if (strcmp(argv[i], "--port") == 0 && PORT != -1) {
            printf("Usage: './ask4 [--host HOST] [--port PORT] [--debug]'\n");
            printf("You entered '--port' twice\n");
            exit(0);
        }
    }

    // If the user did not provide the host and port number
    // then the default values are assigned to them
    if (PORT == -1) PORT = 20241;
    if (strlen(HOST) == 0) snprintf(HOST, 200, "os4.iot.dslab.ds.open-cloud.xyz");

    // We need to get the information about the host
    // in order to connect to it
    // It will get the IP address of the host which we need
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(HOST)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }

    printf("Official name is: %s\n", he->h_name);
    printf("IP addresses: ");
    // type casting the address list that is in 'he'
    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
        //Return the first one;
        // 'inet_ntoa' transforms IPs from their binary form to a string in dotted-decimal notation
        printf("%s ", inet_ntoa(*addr_list[i]));
    }

    printf("\n");

    if (debug) {
        printf("HOST: %s\n", HOST);
        printf("PORT: %d\n", PORT);
    }


    // The rest of the code goes here
    int client_fd;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = addr_list[0]->s_addr;
    
    // Creating a socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Connecting to the server
    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        exit(1);
    }

    fd_set rfds; // The set of file descriptors

    // The return value of the select function
    // which returns the number of file descriptors that are ready
    int retval;

    int max_fd = client_fd + 1;

    while(1) {
        FD_ZERO(&rfds); // Clear the set of file descriptors
        FD_SET(0, &rfds); // Add the standard input to the set of file descriptors
        FD_SET(client_fd, &rfds); // Add the client file descriptor to the set of file descriptors 
        // Wait for an input on either the standard input or the client file descriptor
        // The last parameter is NULL because we don't want a timeout, we want to wait indefinitely
        retval = select(max_fd, &rfds, NULL, NULL, NULL);
        // printf("retval: %d\n", retval);

        if (retval < 0) {
            perror("select failed");
            exit(1);
        }


        // Check if there is an input on the standard input
        // FD_ISSET returns a non-zero value if the file descriptor is ready
        if (FD_ISSET(0, &rfds)) {
            char usr_input[100];
            fgets(usr_input, 100, stdin); // Third parameter is the standard input file descriptor
            printf("You entered: %s", usr_input);


            if (strcmp(usr_input, "help\n") == 0) {
                if (debug) printf("[DEBUG] User entered 'help'\n");

                printf("The following commands are available:\n");
                printf("help - Display this help message\n");
                printf("exit - Close the connection and exit the program\n");
                printf("get - Retrieve real time sensor data from the server\n");
                printf("N name surname reason - Άδεια εξόδου στην καραντίνα\n");
                continue;
            }
            else if (strcmp(usr_input, "exit\n") == 0) {
                if (debug) printf("[DEBUG] User entered 'exit'\n");

                close(client_fd);
                exit(0);
            }
            else if (strcmp(usr_input, "get\n") == 0) {
                if (debug) printf("[DEBUG] User entered 'get'\n");

                // We add 1 to the length to send the null character as well
                write(client_fd, "get", 4);

                if (debug) printf("[DEBUG] Sent 'get' to the server\n");
            }
            // It is not checking for the validity of the input
            // Forwards the responsibility of checking to the server
            else {
                // Remove the newline character from the input
                usr_input[strlen(usr_input) - 1] = '\0';

                if (debug) printf("[DEBUG] User entered '%s'\n", usr_input);

                // We add 1 to the length to send the null character as well
                write(client_fd, usr_input, strlen(usr_input) + 1);

                if (debug) printf("[DEBUG] Sent '%s' to the server\n", usr_input);
            }
        }

        if (FD_ISSET(client_fd, &rfds)) {
            // Initialize the array with null-termination characters
            char server_response[100] = {0};
            ssize_t num_bytes_read = read(client_fd, server_response, sizeof(server_response) - 1);
            if (num_bytes_read < 0) {
                perror("read failed");
                exit(1);
            }
            else if (num_bytes_read == 0) {
                printf("The server has closed the connection\n");
                close(client_fd);
                exit(0);
            }

            if (strcmp(server_response, "invalid code\n") == 0 || strcmp(server_response, "try again\n") == 0) {
                // Remove the newline character from the end of the response
                server_response[strlen(server_response) - 1] = '\0';

                if (debug) printf("[DEBUG] Received '%s' from the server\n", server_response);

                printf("%s\n", server_response);
            }
            // The length of the response when 'get' is sent to the server
            // is always 22 and other responses from the server do not have
            // a length of 22 so we can safely assume when the length of
            // the response is 22, it is the real time sensor data
            else if (strlen(server_response) == 22) {
                server_response[21] = '\0'; // Remove the newline character from the end of the response
                if (debug) printf("[DEBUG] Received '%s' from the server\n", server_response);

                long light_level = strtol(server_response + 2, NULL, 10);
                float temperature = ((double) strtol(server_response + 6, NULL, 10)) / 100;
                long timestamp = strtol(server_response + 11, NULL, 10);
                time_t rawtime = (time_t) timestamp;

                struct tm *time_info;
                time_info = localtime(&rawtime);

                char formatted_time[100] = {0};
                strftime(formatted_time, 100, "%Y-%m-%d %H:%M:%S", time_info);


                printf("--------------------------------\n");
                printf("Latest event:\n");
                printf("interval (%c)\n", server_response[0]);
                printf("Light level is: %ld\n", light_level);
                printf("Temperature is: %0.2f\n", temperature);
                printf("Timestamp is: %s\n", formatted_time);
            }
            else if (
                server_response[0] == 'A' &&
                server_response[1] == 'C' &&
                server_response[2] == 'K' &&
                server_response[3] == ' '
            ) {
                if (debug) printf("[DEBUG] Received '%s' from the server\n", server_response);

                printf("Response: '%s'\n", server_response);
            }
            else {
                if (debug) printf("[DEBUG] Received '%s' from the server\n", server_response);

                printf("Send verification code: '%s'\n", server_response);
            }
        }

    }

    return 0;
}
