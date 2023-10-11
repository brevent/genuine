//
// Created by Thom on 2019/3/8.
//

#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef MAIN
#include <stdio.h>
#else

#include "common.h"
#include "openat.h"

#endif

#include "apk-sign-v2.h"

static bool isApkSigBlock42(const char *buffer) {
    // APK Sig Block 42
    return *buffer == 'A'
           && *++buffer == 'P'
           && *++buffer == 'K'
           && *++buffer == ' '
           && *++buffer == 'S'
           && *++buffer == 'i'
           && *++buffer == 'g'
           && *++buffer == ' '
           && *++buffer == 'B'
           && *++buffer == 'l'
           && *++buffer == 'o'
           && *++buffer == 'c'
           && *++buffer == 'k'
           && *++buffer == ' '
           && *++buffer == '4'
           && *++buffer == '2';
}

int checkSignature(const char *path) {
    unsigned char buffer[0x11] = {0};
    uint32_t size4;
    uint64_t size8, size_of_block;

#ifdef DEBUG
    LOGI("check signature for %s", path);
#endif

    int sign = -1;
#ifdef MAIN
#define openAt openat
#endif
    int fd = (int) openAt(AT_FDCWD, path, O_RDONLY);
#ifdef DEBUG_OPENAT
    LOGI("openat %s returns %d", path, fd);
#endif
    if (fd < 0) {
        return sign;
    }

    bool verified = false;
    sign = 1;
    // https://en.wikipedia.org/wiki/Zip_(file_format)#End_of_central_directory_record_(EOCD)
    for (int i = 0;; ++i) {
        unsigned short n;
        lseek(fd, -i - 2, SEEK_END);
        read(fd, &n, 2);
        if (n == i) {
            lseek(fd, -22, SEEK_CUR);
            read(fd, &size4, 4);
            if ((size4 ^ 0xcafebabeu) == 0xccfbf1eeu) {
#ifdef MAIN
                if (i > 0) {
                    printf("warning: comment length is %d\n", i);
                }
#endif
                break;
            }
        }
        if (i == 0xffff) {
#ifdef MAIN
            printf("error: cannot find eocd\n");
#endif
            goto clean;
        }
    }

    lseek(fd, 12, SEEK_CUR);
    // offset
    read(fd, &size4, 0x4);
    lseek(fd, (off_t) (size4 - 0x18), SEEK_SET);

    read(fd, &size8, 0x8);
    read(fd, buffer, 0x10);
    if (!isApkSigBlock42((char *) buffer)) {
        goto clean;
    }

    lseek(fd, (off_t) (size4 - (size8 + 0x8)), SEEK_SET);
    read(fd, &size_of_block, 0x8);
    if (size_of_block != size8) {
        goto clean;
    }

    for (;;) {
        uint32_t id;
        uint32_t offset;
        read(fd, &size8, 0x8); // sequence length
        if (size8 == size_of_block) {
            break;
        }
        read(fd, &id, 0x4); // id
        offset = 4;
#ifdef MAIN
        printf("id: 0x%08x\n", id);
#endif
        if ((id ^ 0xdeadbeefu) == 0xafa439f5u) {
            sign = 2;
        } else if ((id ^ 0xdeadbeefu) == 0x2efed62f) {
            sign = 3;
        } else if ((id ^ 0xdeadbeefu) == 0xc53e138eu) {
            sign = 31;
        } else {
            sign = 1;
        }
        if (sign > 1) {
            verified = false;
            read(fd, &size4, 0x4); // signer-sequence length
            read(fd, &size4, 0x4); // signer length
            read(fd, &size4, 0x4); // signed data length
            offset += 0x4 * 3;

            read(fd, &size4, 0x4); // digests-sequence length
            lseek(fd, (off_t) (size4), SEEK_CUR);// skip digests
            offset += 0x4 + size4;

            read(fd, &size4, 0x4); // certificates length
            read(fd, &size4, 0x4); // certificate length
            offset += 0x4 * 2;
            int hash = 1;
            signed char c;
            for (unsigned i = 0; i < size4; ++i) {
                read(fd, &c, 0x1);
                hash = 31 * hash + c;
            }
            offset += size4;
#ifdef MAIN
            printf("    size: 0x%04x, hash: 0x%08x, v%d\n", size4, ((unsigned) hash) ^ 0x14131211u, sign);
#else
#if defined(GENUINE_SIZE) && defined(GENUINE_HASH)
            if (size4 == GENUINE_SIZE && ((((unsigned) hash) ^ 0x14131211u) == GENUINE_HASH)) {
                verified = true;
            }
#endif
#if defined(GENUINE_SIZE_31) && defined(GENUINE_HASH_31)
            if (sign == 31 && size4 == GENUINE_SIZE_31 && ((((unsigned) hash) ^ 0x14131211u) == GENUINE_HASH_31)) {
                verified = true;
            }
#else
            // ignore check apk sign v31
            if (sign == 31) {
                verified = true;
            }
#endif
            if (!verified) {
                break;
            }
#endif
        }
        lseek(fd, (off_t) (size8 - offset), SEEK_CUR);
    }

clean:
    close(fd);

    if (verified) {
        return 0;
    }
    return sign;
}

#ifdef MAIN
int main(int argc, char **argv) {
    if (argc > 1) {
        checkSignature(argv[1]);
    }
}
#endif
