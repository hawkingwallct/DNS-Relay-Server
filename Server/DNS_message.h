// DNS查询类型的定义
#define A 1     // A记录
#define CNAME 5 // CNAME记录

/*
** DNS消息头部
** 使用位字段表示
*/
typedef struct
{
    unsigned short id;         // 会话标识
    unsigned char rd : 1;      // 表示是否递归查询
    unsigned char tc : 1;      // 表示是否可截断的
    unsigned char aa : 1;      // 表示授权应答
    unsigned char opcode : 4;  // 操作码
    unsigned char qr : 1;      // 查询/响应标志，0为查询，1为响应
    unsigned char rcode : 4;   // 响应码
    unsigned char cd : 1;      // 检查禁用
    unsigned char ad : 1;      // 认证数据
    unsigned char z : 1;       // 保留位
    unsigned char ra : 1;      // 表示是否可递归
    unsigned short q_count;    // 表示查询问题区域的记录数
    unsigned short ans_count;  // 表示应答区域的记录数
    unsigned short auth_count; // 表示授权区域的记录数
    unsigned short add_count;  // 表示附加区域的记录数
} DNS_HEADER;

/*
** DNS消息中的查询问题部分
*/
typedef struct
{
    unsigned short qtype;  // 查询类型
    unsigned short qclass; // 查询类
} QUESTION;
typedef struct
{
    unsigned char *name;   // 查询域名
    QUESTION *ques; // 查询问题
} QUERY;

/*
** DNS消息中的应答部分
*/
// 保持结构体对齐为1字节
#pragma pack(push, 1) // 保存当前对齐状态，并设置对齐为1字节对齐
typedef struct
{
    unsigned short type;     // 表示资源记录的类型
    unsigned short _class;   // 类
    unsigned int ttl;        // 表示资源记录可以缓存的时间
    unsigned short data_len; // 数据长度
} ANSWER;
#pragma pack(pop) // 恢复之前的对齐状态

/*
** DNS消息中的资源记录部分
*/
typedef struct
{
    unsigned char *name;     // 资源记录的域名
    ANSWER *resource; // 资源数据
    unsigned char *rdata;    // 资源记录的数据
} RES_RECORD;
