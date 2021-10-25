# TCP example 

## build


```bash
make
```

`make` 之後會產生 `tcp_server` 跟 `tcp_client` 兩個執行檔

為了讓資料能正常的傳輸，必須先執行 `tcp_server` 讓 `socket` 去監聽 `port`
再執行 `tcp_client` 傳送資料，執行結果如下

![](https://i.imgur.com/hwQR7X9.png)

![](https://i.imgur.com/ui9e61W.png)

## uninstall

```bash
make clean
```