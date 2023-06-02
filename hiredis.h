/*
 * Copyright (c) 2009-2011, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2014, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2015, Matt Stancliff <matt at genges dot com>,
 *                     Jan-Erik Rediger <janerik at fnordig dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * 这里的内容不必看，是版权说明，只要保留原有这部份就可以了。
 */

#ifndef __HIREDIS_H
#define __HIREDIS_H
#include "read.h"
#include <stdarg.h> /* for va_list */
#ifndef _MSC_VER
#include <sys/time.h> /* for struct timeval */
#else
struct timeval; /* 前置声明 forward declaration 时间类型用来在框架内提供相关时间戳内容*/
typedef long long ssize_t;
#endif
#include <stdint.h> /* uintXX_t, etc */
#include "sds.h"    /* for sds */
#include "alloc.h"  /*对于分配包装 for allocation wrappers */

#define HIREDIS_MAJOR 1
#define HIREDIS_MINOR 1
#define HIREDIS_PATCH 1
#define HIREDIS_SONAME 1.1.1 - dev

/* Connection type can be blocking or non-blocking and is set in the
 * least significant bit of the flags field in redisContext.
 * 连接类型可以阻止或阻塞和设置
 * 在redisContext标志字段的最低有效位。
 */
#define REDIS_BLOCK 0x1

/* Connection may be disconnected before being free'd. The second bit
 * in the flags field is set when the context is connected.
 * 在释放之前链接可能被断开。第二位
 * 在标志字段设置上下文连接。
 */
#define REDIS_CONNECTED 0x2

/* The async API might try to disconnect cleanly and flush the output
 * buffer and read all subsequent replies before disconnecting.
 * This flag means no new commands can come in and the connection
 * should be terminated once all replies have been read.
 * 异步API可能会尝试彻底断开连接，
 * 并在断开连接之前刷新output*缓冲区并读取所有后续回复。
 * 这个标志意味着没有新的命令可以进入，
 * 一旦所有的应答都被读取，连接应该被终止。
 *  */
#define REDIS_DISCONNECTING 0x4

/* Flag specific to the async API which means that the context should be clean
 * up as soon as possible.
 * 特定于异步API的标志，这意味着上下文应该尽快清理。
 * */
#define REDIS_FREEING 0x8

/* Flag that is set when an async callback is executed.
 * 执行异步回调时设置的标志。
 */
#define REDIS_IN_CALLBACK 0x10

/* Flag that is set when the async context has one or more subscriptions.
 * 当异步上下文有一个或多个下标时设置的标志
 */
#define REDIS_SUBSCRIBED 0x20

/* 监视模式处于活动状态时设置的标志 Flag that is set when monitor mode is active */
#define REDIS_MONITORING 0x40

/* 当我们应该在调用bind()之前设置SO_REUSEADDR时设置的标志 Flag that is set when we should set SO_REUSEADDR before calling bind() */
#define REDIS_REUSEADDR 0x80

/* 当异步连接支持推送应答时设置的标志。 Flag that is set when the async connection supports push replies. */
#define REDIS_SUPPORTS_PUSH 0x100

/**
 * Flag that indicates the user does not want the context to
 * be automatically freed upon error
 * 表明用户不希望在出错时自动释放上下文的标志
 */
#define REDIS_NO_AUTO_FREE 0x200

/* 表明用户不希望回复被自动释放的标志 Flag that indicates the user does not want replies to be automatically freed */
#define REDIS_NO_AUTO_FREE_REPLIES 0x400

/* Flags to prefer IPv6 or IPv4 when doing DNS lookup. (If both are set,
 * AF_UNSPEC is used.)
 * 在进行DNS查找时选择IPv6或IPv4的标志。(如果两者都设置了，则使用AF_UNSPEC。)
 * */
#define REDIS_PREFER_IPV4 0x800
#define REDIS_PREFER_IPV6 0x1000

#define REDIS_KEEPALIVE_INTERVAL 15 /* seconds */

/* number of times we retry to connect in the case of EADDRNOTAVAIL and
 * SO_REUSEADDR is being used.
 * 在EADDRNOTAVAIL和SO_REUSEADDR被使用的情况下，我们尝试连接的次数。
 *  */
#define REDIS_CONNECT_RETRIES 10

/* 在别处定义的结构的前向声明 Forward declarations for structs defined elsewhere */
struct redisAsyncContext;
struct redisContext;

/* RESP3推动助手和回调函数原型 RESP3 push helpers and callback prototypes */
#define redisIsPushReply(r) (((redisReply *)(r))->type == REDIS_REPLY_PUSH)
typedef void(redisPushFn)(void *, void *);
typedef void(redisAsyncPushFn)(struct redisAsyncContext *, void *);

#ifdef __cplusplus
extern "C"
{
#endif

    /* 这是返回的应答对象redisCommand ()                                                    This is the reply object returned by redisCommand() */
    typedef struct redisReply
    {
        int type;                    /* REDIS_REPLY_* */
        long long integer;           /* type为REDIS_REPLY_INTEGER时的整数                   he integer when type is REDIS_REPLY_INTEGER */
        double dval;                 /* double类型是REDIS_REPLY_DOUBLE字符串的长度          The double when type is REDIS_REPLY_DOUBLE */
        size_t len;                  /* 字符串的长度                                        Length of string */
        char *str;                   /* 用于REDIS_REPLY_ERROR, REDIS_REPLY_STRING,          Used for REDIS_REPLY_ERROR, REDIS_REPLY_STRING
                                        REDIS_REPLY_VERB, REDIS_REPLY_DOUBLE(除了dval)，    REDIS_REPLY_VERB, REDIS_REPLY_DOUBLE (in additional to dval),
                                        和REDIS_REPLY_BIGNUM                                and REDIS_REPLY_BIGNUM. */
        char vtype[4];               /*用于REDIS_REPLY_VERB，                               Used for REDIS_REPLY_VERB, contains the null
                                       包含以空结束的3个字符的内容类型，如"txt"                terminated 3 character content type, such as "txt". */
        size_t elements;             /* REDIS_REPLY_ARRAY的元素个数                         number of elements, for REDIS_REPLY_ARRAY */
        struct redisReply **element; /* REDIS_REPLY_ARRAY的元素向量                         elements vector for REDIS_REPLY_ARRAY */
    } redisReply;

    redisReader *redisReaderCreate(void);

    /* 默认函数hiredis自由应答对象返回  Function to free the reply objects hiredis returns by default. */
    void freeReplyObject(void *reply);

    /* 根据协议格式命令功能。 Functions to format a command according to the protocol. */
    int redisvFormatCommand(char **target, const char *format, va_list ap);
    int redisFormatCommand(char **target, const char *format, ...);
    long long redisFormatCommandArgv(char **target, int argc, const char **argv, const size_t *argvlen);
    long long redisFormatSdsCommandArgv(sds *target, int argc, const char **argv, const size_t *argvlen);
    void redisFreeCommand(char *cmd);
    void redisFreeSdsCommand(sds cmd);

    enum redisConnectionType
    {
        REDIS_CONN_TCP,
        REDIS_CONN_UNIX,
        REDIS_CONN_USERFD
    };

    struct redisSsl;

#define REDIS_OPT_NONBLOCK 0x01
#define REDIS_OPT_REUSEADDR 0x02
#define REDIS_OPT_NOAUTOFREE 0x04        /* Don't automatically free the async   \
                                          * object on a connection failure, or   \
                                          * other implicit conditions. Only free \
                                          * on an explicit call to disconnect()  \
                                          * or free() */
                                         /* 不要自动释放async        \
                                          * 连接失败时，*对象，或     \
                                          * 其他隐式条件。只免费      \
                                          * 在显式调用disconnect()   \
                                          * 或free() */
#define REDIS_OPT_NO_PUSH_AUTOFREE 0x08  /* Don't automatically intercept and \
                                          * free RESP3 PUSH replies. */
                                         /* 不要自动拦截和\
                                          * 免费RESP3推送回复。*/
#define REDIS_OPT_NOAUTOFREEREPLIES 0x10 /* 不自动自由回答。 Don't automatically free replies. */
#define REDIS_OPT_PREFER_IPV4 0x20       /* 偏向 IPv4在DNS查找 Prefer IPv4 in DNS lookups. */
#define REDIS_OPT_PREFER_IPV6 0x40       /* 偏向 IPv6在DNS查找 Prefer IPv6 in DNS lookups. */
#define REDIS_OPT_PREFER_IP_UNSPEC (REDIS_OPT_PREFER_IPV4 | REDIS_OPT_PREFER_IPV6)

    /* In Unix systems a file descriptor is a regular signed int, with -1
     * representing an invalid descriptor. In Windows it is a SOCKET
     * (32- or 64-bit unsigned integer depending on the architecture), where
     * all bits set (~0) is INVALID_SOCKET.  */
    /* 在Unix系统中，文件描述符是一个带-1的有符号整型
     * 表示无效的描述符。在Windows中，它是一个SOCKET
     * (32位或64位无符号整数，具体取决于体系结构)，其中
     * 所有位设置(~0)是INVALID_SOCKET。*/

#ifndef _WIN32
    typedef int redisFD;
#define REDIS_INVALID_FD -1
#else
#ifdef _WIN64
typedef unsigned long long redisFD; /* SOCKET = 64-bit UINT_PTR */
#else
typedef unsigned long redisFD; /* SOCKET = 32-bit UINT_PTR */
#endif
#define REDIS_INVALID_FD ((redisFD)(~0)) /* INVALID_SOCKET */
#endif

    typedef struct
    {
        /*
         * the type of connection to use. This also indicates which
         * `endpoint` member field to use
         *
         * 使用的连接类型。这也表明
         * 要使用的' endpoint '成员字段
         */
        int type;
        /* 位域的REDIS_OPT_xxx bit field of REDIS_OPT_xxx */
        int options;
        /* 连接操作的超时值。如果为空,没有使用超时 timeout value for connect operation. If NULL, no timeout is used */
        const struct timeval *connect_timeout;
        /* timeout value for commands. If NULL, no timeout is used.  This can be
         * updated at runtime with redisSetTimeout/redisAsyncSetTimeout. */
        /* 命令的超时时间。如果为NULL，则不使用超时。这可以是
         * 在运行时更新redisSetTimeout/redisAsyncSetTimeout。*/
        const struct timeval *command_timeout;
        union
        {
            /** 使用这个字段为tcp / ip连接 use this field for tcp/ip connections */
            struct
            {
                const char *source_addr;
                const char *ip;
                int port;
            } tcp;
            /** 使用这个字段为unix域套接字 use this field for unix domain sockets */
            const char *unix_socket;
            /**
             * use this field to have hiredis operate an already-open
             * file descriptor */
            /*使用这个字段让雇员操作一个已经打开的文件文件描述符*/
            redisFD fd;
        } endpoint;

        /* 可选的用户定义的数据/析构函数 Optional user defined data/destructor */
        void *privdata;
        void (*free_privdata)(void *);

        /* 用户定义的PUSH消息回调  A user defined PUSH message callback */
        redisPushFn *push_cb;
        redisAsyncPushFn *async_push_cb;
    } redisOptions;

/**
 * Helper macros to initialize options to their specified fields.
 * 辅助宏指定字段初始化选项。
 */
#define REDIS_OPTIONS_SET_TCP(opts, ip_, port_) \
    do                                          \
    {                                           \
        (opts)->type = REDIS_CONN_TCP;          \
        (opts)->endpoint.tcp.ip = ip_;          \
        (opts)->endpoint.tcp.port = port_;      \
    } while (0)

#define REDIS_OPTIONS_SET_UNIX(opts, path)   \
    do                                       \
    {                                        \
        (opts)->type = REDIS_CONN_UNIX;      \
        (opts)->endpoint.unix_socket = path; \
    } while (0)

#define REDIS_OPTIONS_SET_PRIVDATA(opts, data, dtor) \
    do                                               \
    {                                                \
        (opts)->privdata = data;                     \
        (opts)->free_privdata = dtor;                \
    } while (0)

    typedef struct redisContextFuncs
    {
        void (*close)(struct redisContext *);
        void (*free_privctx)(void *);
        void (*async_read)(struct redisAsyncContext *);
        void (*async_write)(struct redisAsyncContext *);

        /* Read/Write data to the underlying communication stream, returning the
         * number of bytes read/written.  In the event of an unrecoverable error
         * these functions shall return a value < 0.  In the event of a
         * recoverable error, they should return 0. */
        /* 读/写数据到底层通信流，返回
         * 读/写字节数。在发生不可恢复的错误时
         * 这些函数应返回值< 0。如果发生
         * 可恢复的错误，它们应该返回0。*/
        ssize_t (*read)(struct redisContext *, char *, size_t);
        ssize_t (*write)(struct redisContext *);
    } redisContextFuncs;

    /* 连接到Redis的上下文 上下文连接复述 Context for a connection to Redis */
    typedef struct redisContext
    {
        const redisContextFuncs *funcs; /* 函数表 Function table */

        int err;          /* 错误标志，当没有错误时为0 Error flags, 0 when there is no error */
        char errstr[128]; /* 适用时，错误的字符串表示形式 String representation of error when applicable */
        redisFD fd;
        int flags;
        char *obuf;          /* Write buffer */
        redisReader *reader; /* 协议的读者 Protocol reader */

        enum redisConnectionType connection_type;
        struct timeval *connect_timeout;
        struct timeval *command_timeout;

        struct
        {
            char *host;
            char *source_addr;
            int port;
        } tcp;

        struct
        {
            char *path;
        } unix_sock;

        /* 对于非阻塞连接 For non-blocking connect */
        struct sockaddr *saddr;
        size_t addrlen;

        /* Optional data and corresponding destructor users can use to provide
         * context to a given redisContext.  Not used by hiredis. */
        /* 用户可以使用可选数据和相应的析构函数来提供
         * context到一个给定的redisContext。不被雇佣者使用。*/
        void *privdata;
        void (*free_privdata)(void *);

        /* Internal context pointer presently used by hiredis to manage
         * SSL connections. */
        /* 内部上下文指针，目前使用hiredis管理
         * SSL连接。*/
        void *privctx;

        /* 一个可选的RESP3 PUSH处理程序 An optional RESP3 PUSH handler */
        redisPushFn *push_cb;
    } redisContext;

    redisContext *redisConnectWithOptions(const redisOptions *options);
    redisContext *redisConnect(const char *ip, int port);
    redisContext *redisConnectWithTimeout(const char *ip, int port, const struct timeval tv);
    redisContext *redisConnectNonBlock(const char *ip, int port);
    redisContext *redisConnectBindNonBlock(const char *ip, int port,
                                           const char *source_addr);
    redisContext *redisConnectBindNonBlockWithReuse(const char *ip, int port,
                                                    const char *source_addr);
    redisContext *redisConnectUnix(const char *path);
    redisContext *redisConnectUnixWithTimeout(const char *path, const struct timeval tv);
    redisContext *redisConnectUnixNonBlock(const char *path);
    redisContext *redisConnectFd(redisFD fd);

    /**
     * Reconnect the given context using the saved information.
     *
     * This re-uses the exact same connect options as in the initial connection.
     * host, ip (or path), timeout and bind address are reused,
     * flags are used unmodified from the existing context.
     *
     * Returns REDIS_OK on successful connect or REDIS_ERR otherwise.
     */
    /**
     * 使用保存的信息重新连接给定的上下文。
     *
     * 这将重复使用与初始连接完全相同的连接选项。
     * 主机，IP(或路径)，超时和绑定地址被重用，
     * 标志在不修改现有上下文的情况下使用。
     *
     * 连接成功返回REDIS_OK，否则返回REDIS_ERR。
     */

    int redisReconnect(redisContext *c);

    redisPushFn *redisSetPushCallback(redisContext *c, redisPushFn *fn);
    int redisSetTimeout(redisContext *c, const struct timeval tv);
    int redisEnableKeepAlive(redisContext *c);
    int redisEnableKeepAliveWithInterval(redisContext *c, int interval);
    void redisFree(redisContext *c);
    redisFD redisFreeKeepFd(redisContext *c);
    int redisBufferRead(redisContext *c);
    int redisBufferWrite(redisContext *c, int *done);

    /* In a blocking context, this function first checks if there are unconsumed
     * replies to return and returns one if so. Otherwise, it flushes the output
     * buffer to the socket and reads until it has a reply. In a non-blocking
     * context, it will return unconsumed replies until there are no more. */
    /* 在阻塞上下文中，该函数首先检查是否有未消耗的内存
     * 回复return，如果返回则返回1。否则，刷新输出
     * 缓冲区到套接字并读取，直到它得到回复。在非阻塞状态下
     * 上下文，它将返回未使用的回复，直到没有回复为止。*/
    int redisGetReply(redisContext *c, void **reply);
    int redisGetReplyFromReader(redisContext *c, void **reply);

    /* Write a formatted command to the output buffer. Use these functions in blocking mode
     * to get a pipeline of commands. */
    /* 写入格式化命令到输出缓冲区。在阻塞模式下使用这些函数
     * 获取命令的管道。*/
    int redisAppendFormattedCommand(redisContext *c, const char *cmd, size_t len);

    /* Write a command to the output buffer. Use these functions in blocking mode
     * to get a pipeline of commands. */
    /* 将命令写入输出缓冲区。在阻塞模式下使用这些函数
     * 获取命令的管道。* /
    int redisvAppendCommand(redisContext *c, const char *format, va_list ap);
    int redisAppendCommand(redisContext *c, const char *format, ...);
    int redisAppendCommandArgv(redisContext *c, int argc, const char **argv, const size_t *argvlen);

    /* Issue a command to Redis. In a blocking context, it is identical to calling
     * redisAppendCommand, followed by redisGetReply. The function will return
     * NULL if there was an error in performing the request, otherwise it will
     * return the reply. In a non-blocking context, it is identical to calling
     * only redisAppendCommand and will always return NULL. */
    /* 向Redis发出命令。在阻塞上下文中，它与调用相同
     * redisAppendCommand，后面跟着redisGetReply。函数将返回
     * 如果在执行请求时出现错误，则返回NULL，否则返回
     * 返回回复。在非阻塞上下文中，它与调用相同
     * 仅redisAppendCommand，并将始终返回NULL。*/
    void *redisvCommand(redisContext *c, const char *format, va_list ap);
    void *redisCommand(redisContext *c, const char *format, ...);
    void *redisCommandArgv(redisContext *c, int argc, const char **argv, const size_t *argvlen);

#ifdef __cplusplus
}
#endif

#endif
