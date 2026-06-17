#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>

void compute_sha256(const char *input, size_t len, char output[65]);

#endif
