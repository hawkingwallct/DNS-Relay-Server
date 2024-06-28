#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "DNS_message.h"

#define SERVER_PORT 53
#define BUFFER_SIZE 65536
#define MAX_DOMAIN_SIZE 256


void parse_query(unsigned char *buffer, char *domain, int sockfd, struct sockaddr_in serv_addr)
{
    // Placeholder for parsing logic
    QUERY *query = (QUERY *)buffer;
    unsigned char *qname = query->name;

    int i = 0, j = 0, k = 0;
    while (qname[i] != 0)
    {
        if (i != 0)
        {
            domain[j++] = '.';
        }
        for (k = 0; k < qname[i]; k++)
        {
            domain[j++] = qname[i + k + 1];
        }
        i += k + 1;
    }
    domain[j] = '\0';
    printf("Query domain: %s\n", domain);

    // Sending the query to the internet DNS server
    if (sendto(sockfd, buffer, sizeof(DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(QUESTION), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int sock_r, rec;
    struct sockaddr_in relayaddr, servaddr;
    socklen_t len;
    unsigned char buffer[BUFFER_SIZE];

    // Create socket
    sock_r = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_r < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket with the server address
    memset(&relayaddr, 0, sizeof(relayaddr));
    relayaddr.sin_family = AF_INET;
    relayaddr.sin_addr.s_addr = INADDR_ANY;
    relayaddr.sin_port = htons(SERVER_PORT);

    if (bind(sock_r, (const struct sockaddr *)&relayaddr, sizeof(relayaddr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("DNS Relay Server is running on port %d\n", SERVER_PORT);

    int sock_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_s < 0) {
        perror("Cannot create socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(53); // DNS server port
    servaddr.sin_addr.s_addr = inet_addr("8.8.8.8"); // Google's public DNS server

    while (1)
    {
        len = sizeof(relayaddr);
        rec = recvfrom(sock_r, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&relayaddr, &len);
        if (rec < 0)
        {
            perror("Receive failed");
            continue;
        }
        else
        {
            DNS_HEADER *heder = (DNS_HEADER *)buffer;
            if (heder->qr == 0)
            {
                // Query
                printf("Received a query\n");
                char domain[MAX_DOMAIN_SIZE];
                parse_query(buffer, domain, sock_s, servaddr);
            }
            if(heder->qr == 1)
            {
                // Response
                printf("Received a response\n");
                char ipv4[16];
                parse_answer(buffer, ipv4,sock_r,relayaddr);
            }
        }
        // Parse the query
        // Placeholder for parsing logic

        // Forward the query
        // Placeholder for forwarding logic

        // Receive the response and send it back to the client
        // Placeholder for response handling logic
    }

    return 0;
}
