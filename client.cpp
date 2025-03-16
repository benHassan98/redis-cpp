#include <assert.h>
#include <cerrno>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
const ssize_t k_max_msg = 4096;
int32_t readFull(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = read(fd, buf, n);
    if (rv <= 0) {
      return -1;
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }

  return 0;
}

int32_t writeAll(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = write(fd, buf, n);
    if (rv <= 0) {
      return -1;
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }

  return 0;
}

int32_t query(int fd, const char *text) {
  int32_t wLen = strlen(text);
  char wBuf[wLen + 4];
  memcpy(wBuf, &wLen, 4);
  memcpy(wBuf + 4, text, wLen);

  ssize_t rv = writeAll(fd, wBuf, wLen + 4);
  if (rv) {
    return -1;
  }

  int32_t rLen = 0;
  char lenBuf[4];
  char rBuf[k_max_msg];

  rv = readFull(fd, lenBuf, 4);
  if (rv) {
    return -1;
  }

  memcpy(&rLen, lenBuf, 4);
  if (rLen > k_max_msg) {
    printf("too long\n");
    return -1;
  }

  rv = readFull(fd, rBuf, rLen);
  if (rv) {
    return -1;
  }

  printf("Server says: %d %s\n", rLen, rBuf);
  return rv;
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int connectRes = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (connectRes) {
    return connectRes;
  }
  int32_t err = query(fd, "hello1");
  if (err) {
    goto L_DONE;
  }
  err = query(fd, "hello2");
  if (err) {
    goto L_DONE;
  }
  err = query(fd, "hello3");
  if (err) {
    goto L_DONE;
  }

L_DONE:
  close(fd);
  return 0;
}
