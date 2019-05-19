## 查看百度地址
```
nslookup baidu.com
Server:		127.0.1.1
Address:	127.0.1.1#53

Non-authoritative answer:
Name:	baidu.com
Address: 220.181.57.216
Name:	baidu.com
Address: 111.13.101.208

```
百度有两组IP地址 `220.181.57.216`和`111.13.101.208`

## 本地IP地址
```
ens33     Link encap:以太网
          inet 地址:192.168.157.128  广播:192.168.157.255  掩码:255.255.255.0
          inet6 地址: fe80::ccb8:1414:368a:3a52/64 Scope:Link
lo        Link encap:本地环回
          inet 地址:127.0.0.1  掩码:255.0.0.0
tap0      Link encap:以太网  硬件地址 0a:fa:5e:2c:db:b3
          inet 地址:192.0.2.2  广播:0.0.0.0  掩码:255.255.255.0

```

## 测试
```
gcc client.c -o client
./client baidu.com 80
# 输出
Connect baidu.com:80 fd:3
connected server address = 192.168.157.128:55622
connected peer address = 220.181.57.216:80
```

## 分析
1. `getsockname` 获得本地IP地址和端口号，系统选择未使用端口号
2. `getpeername` 获得对端IP地址和端口号，端口号由应用指定