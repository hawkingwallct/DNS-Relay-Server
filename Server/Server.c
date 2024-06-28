#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "DNS_message.h"

#define SERVER_PORT 12346
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
    int query_size = sizeof(DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(QUESTION);
    if (sendto(sockfd, buffer, query_size, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
}

void parse_response(unsigned char *buffer, char *ipv4, int sockfd, struct sockaddr_in client_addr)
{
    QUERY *query = (QUERY *)buffer;
    unsigned char *qname = query->name;

    // Placeholder for parsing logic
    RES_RECORD *answer = (RES_RECORD *)buffer;
    unsigned char *rname = answer->name;
    unsigned char *rdata = answer->rdata;
    
    int i = 0, j = 0, k = 0;
    while (rdata[i] != 0)
    {
        if (i != 0)
        {
            ipv4[j++] = '.';
        }
        for (int k = 0; k < rdata[i]; k++)
        {
            ipv4[j++] = rdata[i + k + 1];
        }
        i += k + 1;
    }
    ipv4[j] = '\0';
    printf("IPv4 address: %s\n", ipv4);

    // Send the response back to the client
    int response_size = sizeof(DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(QUESTION) + (strlen((const char *)rname) + 1) + sizeof(ANSWER) + (strlen((const char *)rdata) + 1);
    if (sendto(sockfd, buffer, sizeof(DNS_HEADER) , 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    printf("Response sent to client\n");
}

int main()
{
    struct sockaddr_in relayaddr, servaddr, clientaddr;
    socklen_t len;
    unsigned char buffer_q[BUFFER_SIZE]; // 请求报文
    unsigned char buffer_r[BUFFER_SIZE]; // 回复报文

    // Create socket
    int sock_r = socket(AF_INET, SOCK_DGRAM, 0);
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
    if (sock_s < 0)
    {
        perror("Cannot create socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(53);                   // DNS server port
    servaddr.sin_addr.s_addr = inet_addr("8.8.8.8"); // Google's public DNS server

    while (1)
    {
        len = sizeof(clientaddr);
        int query = recvfrom(sock_r, buffer_q, BUFFER_SIZE, 0, (struct sockaddr *)&clientaddr, &len);
        if (query < 0)
        {
            perror("Query receive failed");
            continue;
        }
        else
        {
            printf("Received DNS query (%d bytes): ", query);
            for (int i = 0; i < query; ++i)
            {
                printf("%02x ", (unsigned char)buffer_q[i]);
            }
            printf("\n");
            // Query
            printf("Received a query\n");
            char domain[MAX_DOMAIN_SIZE];
            parse_query(buffer_q, domain, sock_s, servaddr);
        }

        len = sizeof(servaddr);
        int response = recvfrom(sock_s, buffer_r, BUFFER_SIZE, 0, (struct sockaddr *)&servaddr, &len);
        if (response < 0)
        {
            perror("Response receive failed");
            continue;
        }
        else
        {
            // Response
            printf("Received a response\n");
            char ipv4[16];
            parse_response(buffer_r, ipv4, sock_r, clientaddr);
        }
    }

    return 0;
}
