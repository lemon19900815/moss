// helloworld.cpp — strcpy 越界 / 页堆(gflags)验证 demo
// 复刻 MQTTHandler.cpp:1025:
//   strcpy(response->sender, rspContent.sender);
// response->sender 由 AllocMsgRsp(100,...) 只分配 100 字节,
// 而 rspContent.sender 是 118 字符的多设备客户端ID列表。
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring>

static const char* LONG_ID =
    "1234-6789-1234|zgq-2-c200-1|zgq-2-c200-2|zgq-2-u550-1|zgq-2-u550-2|"
    "zgq-2-u650-1|zgq-2-u650-2|zgq-2-u650-3|zgq-2-u650-4";

static const int BUF_SIZE = 100;

int main(int argc, char** argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);   // 无缓冲, 崩溃也不丢输出

    printf("hello world\n");
    printf("source  = %zu chars\n", strlen(LONG_ID));
    printf("buffer  = %d bytes\n", BUF_SIZE);

    char* buf = new char[BUF_SIZE];
    char* neighbor = new char[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    memset(neighbor, 'N', BUF_SIZE);

    printf("buf      @ %p\n", (void*)buf);
    printf("neighbor @ %p (delta = %lld bytes)\n",
           (void*)neighbor, (long long)((char*)neighbor - (char*)buf));
    printf("overflow writes %zu bytes -> %zu bytes past buffer\n",
           strlen(LONG_ID) + 1, strlen(LONG_ID) + 1 - BUF_SIZE);

    if (argc > 1 && strcmp(argv[1], "safe") == 0)
    {
        printf("[safe] checking length before copy...\n");
        if (strlen(LONG_ID) + 1 > BUF_SIZE)
            printf("[safe] copy refused: source %zu > buffer %d (fixed behavior)\n",
                   strlen(LONG_ID) + 1, BUF_SIZE);
        else
            strcpy(buf, LONG_ID);
    }
    else
    {
        printf("calling strcpy(buf, source)...\n");
        strcpy(buf, LONG_ID);   // <== 越界点, 复刻 MQTTHandler.cpp:1025
        printf("strcpy returned, no exception (overflow was silent)\n");
        printf("neighbor[0..23] = %.24s\n", neighbor);
        printf("neighbor is %s\n",
               strncmp(neighbor, "NNNN", 4) == 0 ? "intact" : "OVERWRITTEN by overflow");
    }

    printf("freeing neighbor...\n");
    delete[] neighbor;
    printf("freeing buf...\n");
    delete[] buf;
    printf("program exited normally\n");
    return 0;
}
