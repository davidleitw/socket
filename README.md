# socket programming

`socket` 本質上是一種 **IPC** (`Inter-Process Communication`) 的技術，用於兩個或多個 `process` 進行資料交換或者通訊。

在網路領域，`socket` 著重的不是同一台主機間 `process` 的通訊，而是不同主機執行的 `process` 互相交換資料的通訊。

我們在寫 `socket programming` 的時候會使用 `os` 提供的 `API`，來避免重複造輪子，今天的筆記會簡單介紹一下 `linux` 提供的 `socket API`，並用兩個簡單的範例介紹如何用 `tcp` 跟 `udp` 協定透過 `socket` 傳輸資料。

## socket

```c
int socket(int domain, int type, int protocol)
```

### **domain**
定義要建立哪一種類型的 `socket`，常用的有以下幾種類型
- **AF_UNIX**, **AF_LOCAL**: 用於本機間 `process` 的溝通   
- **AF_INET**, **AF_INET6**
    - **AF_INET**: IPv4 協定
    - **AF_INET6**: IPv6 協定

詳細的選項可以參考 `socket` 的 [man page](https://man7.org/linux/man-pages/man2/socket.2.html)

### **type**
`socket` 傳輸資料的手段(`communication semantics`)

- **SOCK_STREAM**: 對應到 `tcp` 協定
- **SOCK_DGRAM**: 對應到 `udp` 協定

### **protocol**
設定通訊協定的號碼，通常在寫的時候會填入 `0`，`kernel` 會根據上面的兩個參數自動選擇合適的協定。

[protocol man page](https://man7.org/linux/man-pages/man5/protocols.5.html#top_of_page)

`/etc/protocols` 可以看到 `linux` 底下支援的協定

### **Return Value**

成功建立 `socket` 之後，此函式會返回該 `socket` 的**檔案描述符**(`socket file descriptor`)，在之後的操作可以透過這個回傳值來操作我們建立的 `socket`。 如果建立失敗則會回傳 `-1(INVALID_SOCKET)`

### 檔案描述符是什麼?

// TODO

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
    unsigned short int sa_family;
    char sa_data[14];
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
    unsigned short int sin_family;
    unsigned short int sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[8]; // 填充，讓 sockaddr_in 的 size 跟 sockaddr 相同
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