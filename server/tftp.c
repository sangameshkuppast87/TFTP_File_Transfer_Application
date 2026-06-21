#include "tftp.h"

extern char mode[10];
extern int packet_index;

// Function to send the file from the server to the client
void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		return;
	}

	tftp_packet pkt, ack;
	int bytes;
	packet_index = 1;

	do
	{
		memset(&pkt, 0, sizeof(pkt));

		// For the normal mode
		if (strcasecmp(mode, "normal") == 0)
			bytes = read(fd, pkt.body.data_packet.data, 512);

		// For the octet mode
		else if (strcasecmp(mode, "octet") == 0)
			bytes = read(fd, pkt.body.data_packet.data, 1);

		// for the netascii mode
		else if (strcasecmp(mode, "netascii") == 0)
		{
			char buff[512];
			int i = 0;
			char ch;

			while (i < 512 && read(fd, &ch, 1) == 1)
			{
				if (ch == '\n')
					buff[i++] = '\r';

				if (i == 512)
				{
					lseek(fd, -1, SEEK_CUR);
					break;
				}

				buff[i++] = ch;
			}

			memcpy(pkt.body.data_packet.data, buff, i);
			bytes = i;
		}

		pkt.opcode = DATA;
		pkt.body.data_packet.block_number = packet_index;

		// sending the data packet to the client
		sendto(sockfd, &pkt, 4 + bytes, 0, (struct sockaddr *)&client_addr, client_len);

		printf("Sent block %d, bytes = %d\n", packet_index, bytes);

		// waiting for correct ACK
		do
		{
			// Receiving the acknowledgement from the client
			memset(&ack, 0, sizeof(ack));
			recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&client_addr, &client_len);
		} while (ack.opcode != ACK || ack.body.ack_packet.block_number != packet_index);

		packet_index++;

	} while ((strcasecmp(mode, "octet") == 0 && bytes == 1) || (strcasecmp(mode, "normal") == 0 && bytes == 512) || (strcasecmp(mode, "netascii") == 0 && bytes == 512));

	close(fd);
	printf("\n*****FILE SENT SUCCESSFULLY *****\n\n");
}


// Function to recieve file from the client
void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
	int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd < 0)
	{
		perror("open");
		return;
	}

	tftp_packet pkt;
	int bytes;
	packet_index = 1;

	do
	{
		memset(&pkt, 0, sizeof(pkt));

		int n = recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&client_addr, &client_len);

		if (pkt.opcode != DATA || pkt.body.data_packet.block_number != packet_index)
			continue;

		bytes = n - 4;  // removing opcode(2) + block(2)

		// For normal and the netascii mode 
		if (strcasecmp(mode, "normal") == 0 || strcasecmp(mode, "netascii") == 0)
			write(fd, pkt.body.data_packet.data, bytes);

		// For the octet mode
		else if (strcasecmp(mode, "octet") == 0)
			write(fd, pkt.body.data_packet.data, bytes);

		printf("Received block %d, bytes = %d\n", packet_index, bytes);

		memset(&pkt, 0, sizeof(pkt));
		pkt.opcode = ACK;
		pkt.body.ack_packet.block_number = packet_index;

		// Sending the ackonowledgement to the client
		sendto(sockfd, &pkt, 4, 0, (struct sockaddr *)&client_addr, client_len);

		packet_index++;

	} while ((strcasecmp(mode, "octet") == 0 && bytes == 1) || ((strcasecmp(mode, "normal") == 0 || strcasecmp(mode, "netascii") == 0) && bytes == 512));

	close(fd);
	printf("\n***** FILE RECEIVED SUCCESSFULLY *****\n\n");
}