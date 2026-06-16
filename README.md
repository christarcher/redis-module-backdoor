# redis-module-backdoor

基于 https://github.com/n0b0dyCN/RedisModules-ExecuteCommand 二开, 原项目代码写得有问题, 甚至写了sideof(buf)这种写法, 内存不安全. 而开是因为想要设计一个带有密码校验的, 甚至更多功能的"后门", 作为后渗透中权限维持工具使用.

内存马等长期作为EDR监控的重点, 而ssh私钥等又太过于明显, 使用redis来作为后门入口进行权限维持, 是一个比较不错的选择.

```
127.0.0.1:6379> system.exec "123" "id"
"uid=0(root) gid=0(root) groups=0(root)\n"
```

这里选择使用sha256来存储密码, 不考虑加盐存储

只要密码够复杂且没有暴露给cmd5过, 没有办法爆破.

**注意不要用来执行交互式或长时间挂起的指令, redis会阻塞, 导致业务完全崩溃**
