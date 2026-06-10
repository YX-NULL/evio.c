// Copyright 2022 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
// Documentation at https://github.com/tidwall/evio.c

#ifndef EVIO_H
#define EVIO_H

#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

struct evio_conn;/*代表一个活跃的网络连接*/

/*
手动关闭连接
*/
void evio_conn_close(struct evio_conn *conn);
/**
 * 为每个连接绑定/获取用户自定义数据。
*/
/**
 * // 定义连接上下文结构
struct conn_ctx {
    char user_id[32];
    int state;  // 0:未认证, 1:已认证
    char buffer[4096];
    size_t buffer_len;
};

void opened(struct evio_conn *conn, void *udata) {
    // 分配上下文并绑定到连接
    struct conn_ctx *ctx = malloc(sizeof(struct conn_ctx));
    memset(ctx, 0, sizeof(*ctx));
    ctx->state = 0;
    evio_conn_set_udata(conn, ctx);//set
}

void data(struct evio_conn *conn, const void *data, size_t len, void *udata) {
    // 获取上下文
    struct conn_ctx *ctx = evio_conn_udata(conn);//get

    if (ctx->state == 0) {
        // 未认证状态，处理登录逻辑
        if (strncmp(data, "LOGIN ", 6) == 0) {
            strcpy(ctx->user_id, (char*)data + 6);
            ctx->state = 1;
            evio_conn_write(conn, "OK\n", 3);
        } else {
            evio_conn_close(conn);
        }
    } else {
        // 已认证，处理业务数据
        // ...
    }
}

void closed(struct evio_conn *conn, void *udata) {
    // 清理上下文，防止内存泄漏
    struct conn_ctx *ctx = evio_conn_udata(conn);
    free(ctx);
}
 *
*/
void *evio_conn_udata(struct evio_conn *conn);
void evio_conn_set_udata(struct evio_conn *conn, void *udata);

/**
 * 向连接发送数据
*/
void evio_conn_write(struct evio_conn *conn, const void *data, ssize_t len);

/**
 * 获取客户端的地址字符串
 *
*/
/**
 * void opened(struct evio_conn *conn, void *udata) {
        const char *addr = evio_conn_addr(conn);
        printf("新连接来自: %s\n", addr);

        // 可以实现 IP 白名单
        if (strncmp(addr, "192.168.1.", 10) != 0) {
            printf("拒绝非内网连接: %s\n", addr);
            evio_conn_close(conn);
        }
    }
 *
*/
const char *evio_conn_addr(struct evio_conn *conn);

/**
 * 服务器启动
    │
    ▼
serving()  ← 主线程，执行一次
    │
    ▼
┌─────────────────────────────────────┐
│         事件循环开始                  │
└─────────────────────────────────────┘
    │
    ├──► tick() ← 定时触发（主线程）
    │
    ├──► 新连接到达
    │       │
    │       ▼
    │   opened() ← 工作线程
    │       │
    │       ▼
    │   (等待数据)
    │
    ├──► 数据到达
    │       │
    │       ▼
    │   data() ← 工作线程（可多次触发）
    │
    ├──► 连接关闭
    │       │
    │       ▼
    │   closed() ← 工作线程
    │
    └──► 发生错误
            │
            ▼
        error() ← 工作线程或主线程
 *
 *  tick、serving、error 和 sync，其他回调都在工作线程中执行，绝对不要阻塞它们
*/
struct evio_events {
    int64_t (*tick)(void *udata);
    bool (*sync)(void *udata);
    void (*data)(struct evio_conn *conn, const void *data, size_t len, void *udata);/*收到数据时*/
    void (*opened)(struct evio_conn *conn, void *udata);
    void (*closed)(struct evio_conn *conn, void *udata);         /*新连接建立时*/
    void (*serving)(const char **addrs, int naddrs, void *udata);/*开始监听*/
    void (*error)(const char *message, bool fatal, void *udata);
};

/**
 * 返回当前运行的线程数
*/
int evio_nthreads(void);
int64_t evio_now(void);

/**
 * 启动事件循环，在当前线程中运行，阻塞直到退出
*/
void evio_main(const char *addrs[], int naddrs, struct evio_events events, void *udata);

/**
 * 启动多线程事件循环，使用 nthreads 个工作线程。
   参数说明：
        nthreads = 0：使用 CPU 核心数
        nthreads = 1：单线程模式
        nthreads > 1：指定线程数
 *
*/
void evio_main_mt(const char *addrs[], int naddrs, struct evio_events events, void *udata, int nthreads);
void evio_set_allocator(void *(malloc)(size_t), void (*free)(void*));

#endif

