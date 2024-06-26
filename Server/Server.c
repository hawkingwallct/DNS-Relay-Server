#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <unistd.h>

#include <netdb.h>

#define DNS_PORT 12346

int createUdpServer()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DNS_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    char buffer[512]; // DNS query size

    while (1)
    {
        int recvLen = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &addrlen);
        if (recvLen > 0)
        {
            printf("Received DNS query (%d bytes): ", recvLen);
            for (int i = 0; i < recvLen; ++i)
            {
                printf("%02x ", (unsigned char)buffer[i]);
            }
            printf("\n");

            // 解析DNS请求
            ns_msg msg;
            if (ns_initparse((unsigned char *)buffer, recvLen, &msg) < 0)
            {
                fprintf(stderr, "Failed to parse DNS query\n");
                continue;
            }

            ns_rr rr;
            if (ns_parserr(&msg, ns_s_qd, 0, &rr) < 0)
            {
                fprintf(stderr, "Failed to parse resource record\n");
                continue;
            }

            char query_domain[NS_MAXDNAME];
            if (ns_name_ntop(ns_rr_name(rr), query_domain, sizeof(query_domain)) < 0)
            {
                fprintf(stderr, "Failed to convert domain name\n");
                continue;
            }
            printf("Query domain: %s\n", query_domain);

            // 使用提取的域名构造DNS响应
            unsigned char response[NS_PACKETSZ]; // Response buffer
            int response_length;

            // 初始化解析器
            res_init();

            // 发送DNS查询
            response_length = res_query(query_domain, ns_c_in, ns_t_a, response, sizeof(response));

            if (response_length < 0)
            {
                fprintf(stderr, "DNS query failed: %s\n", hstrerror(h_errno));
            }
            else
            {
                // 解析响应并发送回客户端
                sendto(sockfd, response, response_length, 0, (struct sockaddr *)&clientAddr, addrlen);
            }
        }
    }
    return sockfd;
}

int main()
{
    int sockfd = createUdpServer();
    close(sockfd);
    return 0;
}
