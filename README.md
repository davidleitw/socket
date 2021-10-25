# socket programming

`socket` 本質上是一種 **IPC** (`Inter-Process Communication`) 的技術，用於兩個或多個 `process` 進行資料交換或者通訊。

在網路領域，`socket` 著重的不是同一台主機間 `process` 的通訊，而是不同主機執行的 `process` 互相交換資料的通訊。

我們在寫 `socket programming` 的時候會使用 `os` 提供的 `API`，來避免重複造輪子，今天的筆記會簡單介紹一下 `linux` 提供的 `socket API`，並用兩個簡單的範例介紹如何用 `tcp` 跟 `udp` 協定透過 `socket` 傳輸資料。

![](https://i.imgur.com/gXp0tLh.png)

本文章所使用的環境

- ***kernel***: `5.11.0-37-generic`
- ***gcc version***: `gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0`
- ***GNU Make***: `4.2.1`

在寫 `socket` 相關的程式的時候，需要先

```c
#include <arpa/inet.h>  // sockaddr 相關
#include <sys/socket.h>
```

## socket

```c
int socket(int domain, int type, int protocol)
```

#### *domain*
定義要建立哪一種類型的 `socket`，常用的有以下幾種類型
- **AF_UNIX**, **AF_LOCAL**: 用於本機間 `process` 的溝通   
- **AF_INET**, **AF_INET6**
    - **AF_INET**: IPv4 協定
    - **AF_INET6**: IPv6 協定

詳細的選項可以參考 `socket` 的 [man page](https://man7.org/linux/man-pages/man2/socket.2.html)

#### *type*
`socket` 傳輸資料的手段(`communication semantics`)

- **SOCK_STREAM**: 對應到 `tcp` 協定
- **SOCK_DGRAM**: 對應到 `udp` 協定

#### *protocol*
設定通訊協定的號碼，通常在寫的時候會填入 `0`，`kernel` 會根據上面的兩個參數自動選擇合適的協定。

- [protocol man page](https://man7.org/linux/man-pages/man5/protocols.5.html#top_of_page)

`/etc/protocols` 可以看到 `linux` 底下支援的協定

#### *Return Value*

成功建立 `socket` 之後，此函式會返回該 `socket` 的**檔案描述符**(`socket file descriptor`)，在之後的操作可以透過這個回傳值來操作我們建立的 `socket`。 如果建立失敗則會回傳 `-1(INVALID_SOCKET)`

### 檔案描述符是什麼?


參考資料
- [Everything is a file](https://en.wikipedia.org/wiki/Everything_is_a_file)
- [Linux 的 file descriptor 筆記 FD 真的好重要](https://kkc.github.io/2020/08/22/file-descriptor/)
- [Linux 下 socket 通訊所用的 sockfd 怎麼來的](https://www.cnblogs.com/chorm590/p/12745824.html)

### 建立 socket example

```c
#include <stdio.h>
#include <sys/socket.h>

int main() {
    // AF_INET = IPv4
    // SOCK_DGRAM = UDP
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // 檢查是否建立成功
    if (socket_fd < 0) {
        printf("Fail to create a socket.");
    }
    
    // 根據 socker_fd 關閉剛剛創立的 socket
    close(socket_fd);
    return 0;
}
```

接著先來介紹一下 `socket` 中拿來儲存地址的資料結構 **`sockaddr`**

## sockaddr

`sockaddr` 是 `socket` 的通用地址結構，就如同一開始提到的，`socket` 除了在網路領域之外，也可以在很多不同的地方用來通訊。

`sockaddr` 結構，定義如下

```c
typedef unsigned short int sa_family_t;

#define	__SOCKADDR_COMMON(sa_prefix) \
  sa_family_t sa_prefix##family

struct sockaddr {
    __SOCKADDR_COMMON (sa_);	/* Common data: address family and length.  */
    char sa_data[14];		/* Address data.  */
};

// 上面的結構把巨集展開後，等價於下方的資料結構
struct sockaddr {
    unsigned short int sa_family; // 2 bytes
    char sa_data[14];             // 14 bytes
};
```

後來的更新中，為了讓龐大的程式碼可讀性上升，新增了 `sockaddr_in` 的結構用來存取網路相關的應用， `in` 指的是 `internet`，`sockaddr_in` 專門用來存 `IPv4` 的相關地址。

`IPv6` 則是使用 `sockaddr_in6` 結構，在本文章主要會著重在 `IPv4` 相關的範例。

```c
typedef uint32_t in_addr_t; // 4 byte
struct in_addr {
    in_addr_t s_addr;
};

struct sockaddr_in {    
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;			    /* Port number.  */
    struct in_addr sin_addr;		/* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof (struct sockaddr)
			   - __SOCKADDR_COMMON_SIZE
			   - sizeof (in_port_t)
			   - sizeof (struct in_addr)];
};

struct sockaddr_in {
    // sa_family_t sin_family
    unsigned short int sin_family; // 2 bytes
    unsigned short int sin_port;   // 2 bytes
    struct in_addr sin_addr;       // 4 bytes
    unsigned char sin_zero[8];     // 填充，讓 sockaddr_in 的 size 跟 sockaddr 相同
};
```

這邊觀看原始碼會覺得奇怪，為什麼還需要使用 `sin_zero` 來做填充的動作。

原因是很多 `socket` 的 `api`，參數都需要填入 `sockaddr`，`sockaddr_in` 則是後來加入的 `struct`。 今天如果我們 `address` 的資料是用 `sockaddr_in` 來儲存，並且想調用相關的函式時，我們就需要強制轉型。 

假設今天用 `socket` 的場景不是網路，也會有對應的結構來存地址，在呼叫 `socket` 通用的 `api` 時，就可以使用強制轉型的方式，讓不同的結構呼叫同一個函式。

實際範例: [unix](https://man7.org/linux/man-pages/man7/unix.7.html)

在後面的例子中也會實際調用，下方的程式碼可以先作為參考。

```c
#define serverIP
#define serverPort 12000

// 建立一個 sockaddr_in 結構，存著 server 的相關資料
struct sockaddr_in serverAddr = {
    .sin_family = PF_INET,
    .sin_addr.s_addr = inet_addr(serverIP),
    .sin_port = htons(serverPort)
};

bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
```

## 位置轉換相關的函數

一般我們在表示 `ip` 位置時都會寫成人類比較容易讀的形式，像是`125.102.25.62`

以 `ipv4` 來說，`address` 是由4個 `byte`，32個 `bit`所組成，在實務上我們常常需要做字串與實際數值(`uint32_t`)的轉換，`linux` 函式庫提供了一系列輔助位置轉換的 `function`。

一般來說，`address` 的實際數值都會用 `in_addr` 或者 `in_addr_t` 來表示
其本質就是 `uint32_t`，用總共 32 個 `bits` 來表示一個 `IPv4` 的地址
```c
typedef uint32_t in_addr_t; // 4 byte
struct in_addr {
    in_addr_t s_addr;
};
```

常用的有以下這五種

- 只能用在 `IPv4` 的處理
    - inet_addr
    - inet_aton
    - inet_ntoa
- 兼容 `Ipv4` 與 `IPv6`
    - inet_pton
    - inet_ntop

使用前必須先
```c
#include <arpa/inet.h>
```

### inet_addr


```c
in_addr_t inet_addr(const char *cp)
```

**功能**: 將字串轉換成數值表示的 `ip address`

**回傳**: 假如輸入的地址合法，會回傳 `uint32_t` 的數值，若不合法則回傳 `INADDR_NONE`

> INADDR_NODE = 0xFFFFFFFF (32 個 bits 全部填一)

[範例程式: inet_addr_ex.c](https://github.com/davidleitw/socket/blob/master/address/inet_addr_ex.c)

### inet_aton

```c
int inet_aton(const char *string, struct in_addr *addr)
```

**功能**: 將字串轉換成數值表示的 `ip address`

**回傳**: 轉換成功，會回傳一個非零的值，失敗則會回傳 `0`

[範例程式: inet_aton_ex.c](https://github.com/davidleitw/socket/blob/master/address/inet_aton_ex.c)

### inet_ntoa

```c
char *inet_ntoa(struct in_addr)
```

**功能**: 將 `in_addr` 轉換成字串形式的 `ip address`

**回傳**: 如果沒有錯誤，會傳回成功轉換的字串，失敗時則會回傳 `NULL`

[範例程式: inet_ntoa_ex.c](https://github.com/davidleitw/socket/blob/master/address/inet_ntoa_ex.c)

> [可怕的坑](https://blog.hubert.tw/2009/04/18/%E5%BE%9E-inet_ntoa-%E7%9C%8B-thread-safe-%E7%9A%84-api/)

### inet_pton & inet_ntop

```c
const char *inet_pton(int domain, const void *restrict addr, char *restrict str, socklen_t size)
int inet_pton(int domain, const char *restrict str, void *restrict addr)
```

最後這兩個函式是為了因應 `IPv6` 而新增的，除了轉換 `IPv6` 之外，也可以兼容之前 `IPv4` 相關的轉換，本文章主要是介紹 `IPv4` 相關的用法，`IPv6` 的轉換有興趣的可以自己去查資料。

要做 `IPv6` 相關的轉換，要把 `domain` 填入 `AF_INET6` 即可，後面需要搭配 `IPv6` 相關的 `struct`

```c
#include <stdio.h>
#include <arpa/inet.h>

int main()
{
    struct in_addr addr;
    if (inet_pton(AF_INET, "8.8.8.8", &addr.s_addr) == 1) {
        printf("Ip address: %u\n", addr.s_addr);
    }

    char ip_addr[20];
    if (inet_ntop(AF_INET, &addr.s_addr, ip_addr, sizeof(ip_addr))) {
        printf("After inet_ntop function, ip address: %s\n", ip_addr);
    }
}
```

[inet_pton man page](https://man7.org/linux/man-pages/man3/inet_pton.3.html)
[inet_ntop man page](https://man7.org/linux/man-pages/man3/inet_ntop.3.html)

[範例程式碼 inet_ntop_pton_ex.c](https://github.com/davidleitw/socket/blob/master/address/inet_ntop_pton_ex.c)

轉換相關的 `function` 我每個都寫了一個簡單的範例，可以參考 [完整程式碼](https://github.com/davidleitw/socket/tree/master/address)

## bind

上面介紹了創建一個 `socket` 的方式，也簡單的介紹了存放 `address` 的資料結構，一些常用的轉換函式。

接著我們要介紹 `bind`，這個函式可以讓前面創建的 `socket` 實際綁定到本機的某個 `port` 上面，這樣子 `client` 端在送資料到某個 `port` 的時候，我們寫的 `server` 程式才可以在那個 `port` 上面運行，處理資料。

```c
int bind(int sockfd, struct sockaddr *addr, unsigned int addrlen)
```

#### *sockfd*

一開始呼叫 `socket()` 的回傳值

#### *addr*

`sockaddr` 來描述 `bind` 要綁定的 `address` 還有 `port`。

在先前的介紹有簡單提到，實際存放 `ip address` 的是 `sockaddr_in.sin_addr.s_addr`，如果今天不想綁定 `ip address`，而是單單想綁定某個 `port` 的時候，`s_addr` 就要設成 `INADDR_ANY`，通常會出現在你的主機有多個 `ip` 或者 `ip` 不是固定的情況。

[INADDR_ANY 參考](https://blog.csdn.net/qq_26399665/article/details/52932755)

#### *addrlen*

`addr` 結構的 `size`

#### *return*

如果綁定成功就會回傳 `0`，失敗回傳 `-1`

### example
```c
// 建立 socket, 並且取得 socket_fd
int socket_fd = socket(PF_INET , SOCK_DGRAM , 0);
if (socket_fd < 0) {
    printf("Fail to create a socket.");
}
    
// 地址資訊
struct sockaddr_in serverAddr = {
    .sin_family =AF_INET,             // Ipv4
    .sin_addr.s_addr = INADDR_ANY,    // 沒有指定 ip address
    .sin_port = htons(12000)          // 綁定 port 12000
};

// 綁定
// 因為 bind 可以用在不同種類的 socket，所以是用 sockaddr 宣告
// 我們用於網路的 address，是用 sockaddr_in 這個結構
// 在填入的時候要進行強制轉型
// 前面介紹 sockaddr_in 裡面 sin_zero 就是為了讓兩個結構有相同的 size
if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("Bind socket failed!");
    close(socket_fd);
    exit(0);
}

printf("Server ready!\n");
```

## UDP

接下來就要開始編寫我們的第一支 `socket` 程式，`client` 端輸入小寫的英文字串，`server` 端接收到字串後，將其改成大寫並且送回給 `client` 端。 我們一開始將會透過 `UDP` 協定來實現這個任務。

`UDP` 是一種輕量化的協定，只會提供最低限度的服務，跟 `TCP` 相比，`UDP` 是**非連線導向**的協定，兩個 `process` 之間的溝通並不會事先握手，就像下圖所示，`UDP` 的 `client` 端只會接到指令之後送出，並不會在意對方是否有接收到資料，所以又被稱為 **不可靠的資料傳輸**。

![](https://i.imgur.com/B3WjLDE.png)

![](https://i.imgur.com/Sh21hzp.png)


在 `socket` 的 `api` 中，負責 `UDP` 傳送以及接收的 `function` 是 `sendto()`, `recvfrom()`。 因為 `UDP` 協定不需要事先連線，所以只需要有目標 `ip address` 跟 `port` 即可。

### sendto

- [sendto(2) - Linux man page](https://linux.die.net/man/2/sendto)

```c
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
```

#### *sockfd*

`socket` 的文件描述符

#### *buf*

資料本體

#### *len*

資料長度

#### *flags*

一般填入 `0`，想知道詳細參數意義可以參考 [man page](https://linux.die.net/man/2/sendto)

#### *dest_addr*

目標位置相關資訊

#### *addrlen*

`dest_addr` 的 `size`

#### *return value*

傳送成功時回傳具體傳送成功的 `byte` 數，傳送失敗時會回傳 `-1`
並且把錯誤訊息存進 [errno](https://man7.org/linux/man-pages/man3/errno.3.html)

### recvfrom

- [recvfrom(2) - Linux man page](https://linux.die.net/man/2/recvfrom)

```c
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

#### *sockfd*

`socket` 的文件描述符

#### *buf*

接收資料的 `buffer`

#### *len*

資料長度

#### *flags*

一般填入 `0`，想知道詳細參數意義可以參考 [man page](https://linux.die.net/man/2/recvfrom)

#### *src_addr*

資料來源地址，收到訊息之後我們可以一併收到來源地址，透過 `src_addr`，我們才能順利的把處理完的資料發回。

#### *addrlen*

`src_addr` 的 `size`

#### *return value*

接收成功時回傳具體接收成功的 `byte` 數，傳送失敗時會回傳 `-1`
並且把錯誤訊息存進 [errno](https://man7.org/linux/man-pages/man3/errno.3.html)

### demo

![](https://i.imgur.com/sxPuuic.png)

#### sever example

```c
#define serverPort 48763

// message buffer
char buf[1024] = {0};

// 建立 socket
int socket_fd = socket(PF_INET , SOCK_DGRAM , 0);
if (socket_fd < 0){
    printf("Fail to create a socket.");
}

// server 地址
struct sockaddr_in serverAddr = {
    .sin_family = AF_INET,           
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = htons(serverPort)
};

// 將建立的 socket 綁定到 serverAddr 指定的 port
if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("Bind socket failed!");
    close(socket_fd);
    exit(0);
}

struct sockaddr_in clientAddr;
int len = sizeof(clientAddr);
while (1) {
    // 當有人使用 UDP 協定送資料到 48763 port
    // 會觸發 recvfrom()，並且把來源資料寫入 clientAddr 當中
    if (recvfrom(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&clientAddr, &len) < 0) {
        break;
    }
    
    // 收到 exit 指令就關閉 server
    if (strcmp(buf, "exit") == 0) {
        printf("get exit order, closing the server...\n");
        break;
    }
    
    // 將收到的英文字母換成大寫
    char *conv = convert(buf);
    // 顯示資料來源，原本資料，以及修改後的資料
    printf("get message from [%s:%d]: ", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    printf("%s -> %s\n", buf, conv);
    
    // 根據 clientAddr 的資訊，回傳至 client 端
    sendto(socket_fd, conv, sizeof(conv), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
    
    // 清空 message buffer
    memset(buf, 0, sizeof(buf));
    free(conv);
}

// 關閉 socket，並檢查是否關閉成功
if (close(socket_fd) < 0) {
        perror("close socket failed!");
}
```

#### client example

```c
#define serverPort 48763

// message buffer
char buf[1024] = {0};
char recvbuf[1024] = {0};

// 建立 socket
int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
if (socket_fd < 0) {
    printf("Create socket fail!\n");
    return -1;
}

// server 地址
struct sockaddr_in serverAddr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = inet_addr(serverIP),
    .sin_port = htons(serverPort)
};
int len = sizeof(serverAddr);

while (1) {
    // 輸入資料到 buffer
    printf("Please input your message: ");
    scanf("%s", buf);

    // 傳送到 server 端
    sendto(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    // 接收到 exit 指令就退出迴圈
    if (strcmp(buf, "exit") == 0) 
        break;

    // 清空 message buffer
    memset(buf, 0, sizeof(buf));
    
    // 等待 server 回傳轉成大寫的資料
    if (recvfrom(socket_fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&serverAddr, &len) < 0) {
        printf("recvfrom data from %s:%d, failed!", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    }

    // 顯示 server 地址，以及收到的資料
    printf("get receive message from [%s:%d]: %s\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
    // 清空 recv buffer
    memset(recvbuf, 0, sizeof(recvbuf));
}

// 關閉 socket，並檢查是否關閉成功
if (close(socket_fd) < 0) {
        perror("close socket failed!");
}
```

想了解細節，可參考 [完整程式碼](https://github.com/davidleitw/socket/tree/master/udp_example)

在 `/udp_example` 下執行 `make` 即可。

![](https://i.imgur.com/ui9e61W.png)

![](https://i.imgur.com/hwQR7X9.png)

---

不知道各位有沒有注意到，我們正式使用 `socket` 的 `api` 時，關於位置的部份都是使用 `sockaddr` 當傳入的參數，我們在網路領域用的 `sockaddr_in` 在傳入時都要再強制轉型一次。

因為 `socket` 本身除了網路通訊之外有很多別的地方也會使用到，為了統一 `api` 操作，所以函式一律是用 `sockaddr` 作為參數，這樣一來各種不同的 `sockaddr_xx` 系列就可以用同一組 `api`，只需要額外轉型即可。

```c
recvfrom(socket_fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&serverAddr, &len)
```

---

## TCP

接著我們要談談如何用 `socket` 利用 `TCP` 協定來交換資料，首先要知道的是 `TCP` 屬於 **連線導向`Connection-oriented`** 的協定，跟 `UDP` 不同，在雙方交換資料之前必須經過先建立 `TCP connection`，下方是 `socket` 利用 `TCP` 協定溝通的流程圖，可以跟之前提到 `UDP` 的流程圖做一個簡單的對比。

![](https://i.imgur.com/FDOIMj9.png)


先從 `server` 端來解說，跟 `UDP` 相比，可以看到 `bind` 完之後多了 `listen` 跟 `accept` 兩個動作。

當 `server` 端創立的 `socket` 成功 `bind` 某個 `port` 之後，他會開始 `listen` 有沒有人申請連線，在 `listen` 這個 `function` 還可以設定 `backlog`，這個參數可以決定今天我們的 `socket` 最多能同時處理的連線要求，避免同時太多人提出連線需求。

> *backlog*: 在 `server` 端 `accept` 之前最多的排隊數量



`TCP` 協定在建立連線時會經過 **three-way handshake** 流程，下圖是每個流程與 `socket api` 的對應圖。

![](https://i.imgur.com/IK8laxq.png)


當 `client` 呼叫 `connect` 時才會開始發起 **three-way handshake**，當 `connect` 結束時，`client` 與 `server` 基本已經完成了整個流程。

那 `server` 端的 `accept` 具體只是從 `server socket` 維護的 `completed connection queue` 中取出一個已完成交握過程的 `socket`。

在 `kernel` 中每個 `socket` 都會維護兩個不同的 `queue`:

- 未完成連線佇列 (***incomplete connection queue***): FIFO with syn_rcvd state
- 已完成連線佇列 (***complete connection queue***): FIFO with established state

> 所以 accept 根本不參與具體的 ***three-way handshake*** 流程

參考資料 

[socket listen() 分析](https://www.cnblogs.com/codestack/p/11099565.html)

[從 Linux 原始碼看 socket accept](https://www.readfog.com/a/1638167776017354752)


**總結一下**

- `server` 端
    - `listen`: 初始化佇列，準備接受 `connect`
    - `accept`: 從 `complete connection queue` 中取出一個已連線的 `socket`
- `client` 端
    - `connect`: 發起 `three-way handshake`，必須要等 `server` 端開始 `listen` 後才可以使用

## `Client` 端: *connect*

- [connect(2) Linux man page](https://man7.org/linux/man-pages/man2/connect.2.html)

```c
int connect(int sockfd, const struct sockaddr *addr,
            socklen_t addrlen);
```

#### *sockfd* 

一開始呼叫 `socket()` 的回傳值

#### *addr*

想要建立連線的 `server` 資料

#### *addrlen*

`addr` 結構的 `size`

#### *return*

錯誤時回傳 `-1`，並且設定 `errno`

## `Server` 端: *listen*

- [listen(2) - Linux man page](https://man7.org/linux/man-pages/man2/listen.2.html)

```c
int listen(int sockfd, int backlog);
```

#### *sockfd*

一開始呼叫 `socket()` 的回傳值

#### *backlog*

允許進入 `queue` 的最大連線數量

在 `server` 端還沒有 `accept` 之前，最多能允許幾個 `socket` 申請 `connect`

> 詳細敘述可以參考 [man page](https://man7.org/linux/man-pages/man2/listen.2.html)

#### *return*

錯誤時回傳 `-1`，並且設定 `errno`

## `Server` 端: *accept*

- [accept(2) Linux man page](https://man7.org/linux/man-pages/man2/accept.2.html)

```c
int accept(int sockfd, struct sockaddr *restrict addr,
           socklen_t *restrict addrlen);
```

#### *sockfd*

`server` 端 `socket` 的檔案描述符

#### *addr*

建立 `TCP` 連線的 `Client` 端資料

#### *addrlen* 

`addr` 結構的 `size`

#### *return*

返回一個新的 `sock_fd`，專門跟請求連結的 `client` 互動

### demo

![](https://i.imgur.com/T35C7vs.png)

#### server example

```c
#define serverPort 48763

// message buffer
char buf[1024] = {0};

// 建立 socket
int socket_fd = socket(PF_INET , SOCK_STREAM , 0);
if (socket_fd < 0){
    printf("Fail to create a socket.");
}

// server 地址
struct sockaddr_in serverAddr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = htons(serverPort)
};

// 將建立的 socket 綁定到 serverAddr 指定的 port
if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("Bind socket failed!");
    close(socket_fd);
    exit(0);

// 初始化，準備接受 connect
// backlog = 5，在 server accept 動作之前，最多允許五筆連線申請
// 回傳 -1 代表 listen 發生錯誤
if (listen(socket_fd, 5) == -1) {
    printf("socket %d listen failed!\n", socket_fd);
    close(socket_fd);
    exit(0);
}

printf("server [%s:%d] --- ready\n", 
        inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

while(1) {
    int reply_sockfd;
    struct sockaddr_in clientAddr;
    int client_len = sizeof(clientAddr);

    // 從 complete connection queue 中取出已連線的 socket
    // 之後用 reply_sockfd 與 client 溝通
    reply_sockfd = accept(socket_fd, (struct sockaddr *)&clientAddr, &client_len);
    printf("Accept connect request from [%s:%d]\n", 
            inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    
    // 不斷接收 client 資料
    while (recv(reply_sockfd, buf, sizeof(buf), 0)) {
        // 收到 exit 指令就離開迴圈
        if (strcmp(buf, "exit") == 0) {
            memset(buf, 0, sizeof(buf));
            goto exit;
        }

        // 將收到的英文字母換成大寫
        char *conv = convert(buf);

        // 顯示資料來源，原本資料，以及修改後的資料
        printf("get message from [%s:%d]: ",
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        printf("%s -> %s\n", buf, conv);

        // 傳回 client 端
        // 不需要填入 client 端的位置資訊，因為已經建立 TCP 連線
        if (send(reply_sockfd, conv, sizeof(conv), 0) < 0) {
            printf("send data to %s:%d, failed!\n", 
                    inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            memset(buf, 0, sizeof(buf));
            free(conv);
            goto exit;
        }

        // 清空 message buffer
        memset(buf, 0, sizeof(buf));
        free(conv);
    }

    // 關閉 reply socket，並檢查是否關閉成功
    if (close(reply_sockfd) < 0) {
        perror("close socket failed!");
    }
}
```

#### client example

```c
#define serverPort 48763

 // message buffer
char buf[1024] = {0};
char recvbuf[1024] = {0};

// 建立 socket
int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
if (socket_fd < 0) {
    printf("Create socket fail!\n");
    return -1;
}

// server 地址
struct sockaddr_in serverAddr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = inet_addr(serverIP),
    .sin_port = htons(serverPort)
};
int len = sizeof(serverAddr);

// 試圖連結 server，發起 tcp 連線
// 回傳 -1 代表 server 可能還沒有開始 listen
if (connect(socket_fd, (struct sockaddr *)&serverAddr, len) == -1) {
    printf("Connect server failed!\n");
    close(socket_fd);
    exit(0);
}

printf("Connect server [%s:%d] success\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

while (1) {
    // 輸入資料到 buffer
    printf("Please input your message: ");
    scanf("%s", buf);

    // 傳送到 server 端
    if (send(socket_fd, buf, sizeof(buf), 0) < 0) {
        printf("send data to %s:%d, failed!\n", 
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
        memset(buf, 0, sizeof(buf));
        break;
    }

    // 接收到 exit 指令就退出迴圈
    if (strcmp(buf, "exit") == 0)
        break;

    // 清空 message buffer
    memset(buf, 0, sizeof(buf));

    // 等待 server 回傳轉成大寫的資料
    if (recv(socket_fd, recvbuf, sizeof(recvbuf), 0) < 0) {
        printf("recv data from %s:%d, failed!\n", 
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
        break;
    }

    // 顯示 server 地址，以及收到的資料
    printf("get receive message from [%s:%d]: %s\n", 
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
    memset(recvbuf, 0, sizeof(recvbuf));
}

// 關閉 socket，並檢查是否關閉成功
if (close(socket_fd) < 0) {
    perror("close socket failed!");
}
```
![](https://i.imgur.com/S5sMq9b.png)

使用

```bash
netstat -a | grep 48763
```

查看是否建立連線

![](https://i.imgur.com/hnhnqG9.png)



## localhost

本地端測試網路程式的時候常會使用的地址

可以參考 [wiki](https://zh.wikipedia.org/wiki/Localhost)



### 參考書籍

#### UNIX Network Programming

![](https://i.imgur.com/XDmFV5u.png)

#### TCP/IP Illustrated

![](https://i.imgur.com/DICEwYi.png)


### reference
- [TCP Socket Programming 學習筆記](http://zake7749.github.io/2015/03/17/SocketProgramming/)
- [地址轉換函數 inet_addr(), inet_aton(), inet_ntoa()和inet_ntop(), inet_pton()](http://haoyuanliu.github.io/2017/01/15/%E5%9C%B0%E5%9D%80%E8%BD%AC%E6%8D%A2%E5%87%BD%E6%95%B0inet-addr-inet-aton-inet-ntoa-%E5%92%8Cinet-ntop-inet-pton/)
- [Beej's guide to networking programming](https://beej-zhtw-gitbook.netdpi.net/dao_du)
- [UDP Server-Client implementation in C](https://www.geeksforgeeks.org/udp-server-client-implementation-c/)