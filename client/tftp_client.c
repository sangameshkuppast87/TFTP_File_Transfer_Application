#include "tftp.h"
#include "tftp_client.h"


int packet_index = 1;
char mode[10] = "normal";
int is_connected = 0;

int main()
{
    char command[256];
    tftp_client_t client;
    memset(&client, 0, sizeof(client));  // Initialize client structure

    // Main loop for command-line interface
    while (1)
    {
        printf("TFTP menu:\n");
        printf("1. connect\n2. get\n3. put\n4. mode\n5. exit\n\nEnter your option: ");
        fgets(command, sizeof(command), stdin);

        // Remove newline character
        command[strcspn(command, "\n")] = 0;

        // Process the command
        process_command(&client, command);
        printf("\n");
    }

    return 0;
}

// Function to process commands
void process_command(tftp_client_t *client, char *command)
{
    tftp_packet pkt;
    memset(&pkt, 0, sizeof(pkt));

    if(strcasecmp(command, "connect") == 0)
    {
        char ip_address[20];
        printf("Enter the IP address: ");
        fgets(ip_address, sizeof(ip_address), stdin);
        ip_address[strcspn(ip_address, "\n")] = '\0';
        
        struct sockaddr_in tmp;
        
        //Checking if the user given address is valid or not
        if(inet_pton(AF_INET, ip_address, &(tmp.sin_addr)) == 0)
        {
            printf("Error: Invalid IP address format!!\nExample: 127.0.0.1\n");
            return;
        }
        else
        {
            connect_to_server(client, ip_address, PORT);
            printf("INFO: Connection to server successful!!\n");
        }
        is_connected = 1;
    }

    else if(strcasecmp(command, "put") == 0)
    {
        if(is_connected == 1)
        {
            char filename[30];
            printf("Enter the file name: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = '\0';

            int fd = open(filename, O_CREAT | O_EXCL, 0644);
            if(fd != 1)
            {
                unlink(filename);
                printf("Error: Given file is not present!!\n");
                return;
            }
            else
            {
                put_file(client, filename);
            }
        }
        else
        {
            printf("\nConnection not established with server!!!\nFirst connect with the server\n");
            return;
        }
    }

    else if(strcasecmp(command, "get") == 0)
    {
        if(is_connected == 1)
        {
            char filename[20];
            printf("Enter the filename you want to receive: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = '\0';

            get_file(client, filename);
        }
        else
        {
            printf("\nConnection not established with server!!!\nFirst connect with the server\n");
            return;
        }
    }

    else if(strcasecmp(command, "mode") == 0)
    {
        printf("MENU:\n1. Normal\n2. Octet\n3. Netascii\n");
        printf("Enter the option: ");
        fgets(mode, sizeof(mode), stdin);
        mode[strcspn(mode, "\n")] = '\0';

        printf("CURRENT MODE = %s\n", mode);
    }

    else if(strcasecmp(command, "exit") == 0)
    {
        // Calling the disconnect function 
        printf("Disconnecting from the server!!!\n");
        disconnect(client);
        printf("Exiting......\n");
        exit(0);
    }
    else
    {
        printf("\nError: Invalid option!!\n");
        return;
    }
}

// The function to connect the client with the server 
void connect_to_server(tftp_client_t *client, char *ip, int port)
{
    // Creating UDP socket
    client -> sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(client -> sockfd == -1)
    {
        perror("socket");
        return;
    }

    // setting up server address
    client -> server_addr.sin_family = AF_INET;
    client -> server_addr.sin_port = htons(port);
    client -> server_addr.sin_addr.s_addr = inet_addr(ip);
    client -> server_len = sizeof(client -> server_addr);


}
// Function to put the file
void put_file(tftp_client_t *client, char *filename)
{
    // Calling the send_request function to send the write request
    send_request(client -> sockfd, client -> server_addr, filename, WRQ);
}

// Function to get the file
void get_file(tftp_client_t *client, char *filename)
{
   // Calling the send the request function to send the read request
   send_request(client -> sockfd, client -> server_addr, filename, RRQ);
}

// Function to disconnect the client from the server
void disconnect(tftp_client_t *client)
{
    //close fd
    close(client -> sockfd);  
}

// Function to send the request 
void send_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode)
{
    //updating the packet number everytime while sending new data
    packet_index = 1;

    tftp_packet pkt;
    
    // clearing the garbage values 
    memset(&pkt, 0, sizeof(pkt));

    /* initializing the variables */
    pkt.opcode = opcode;
    strcpy(pkt.body.request.filename, filename);

    // Copying the mode
    strcpy(pkt.body.request.mode, mode);
    
    // sending request to the server
    sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    //calling the receive_request function
    receive_request(sockfd, server_addr, filename, opcode);

}

// Function to receive the request from the server
void receive_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode)
{
    tftp_packet pkt;
    
    //clearing the garbage values
    memset(&pkt, 0, sizeof(pkt));
    
    socklen_t len = sizeof(server_addr);
    
    // receiving the acknowledgement
    recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, &len);

    //checking opcode for the condition
    if(pkt.opcode == ACK)
    {
        /* Write request received so sending the file */
        if(pkt.body.ack_packet.block_number == WRITE)
        {
            printf("\n***** READY TO SEND *****\n\n");
            send_file(sockfd, server_addr, len, filename);
        }

        //Read request received so receiving the file
        else if(pkt.body.ack_packet.block_number == READ)
        {
            printf("\n***** READY TO RECEIVE *****\n\n");
            receive_file(sockfd, server_addr, len, filename);
        }
    }
    // This is for ERROR
    else if(pkt.opcode == ERROR)
    {
        printf("\nError: \n");
        printf("%s\n\n", pkt.body.error_packet.error_msg);
    }
}