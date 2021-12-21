#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

#include "./layer1.h"
#include "./gen_crc16.h"

typedef struct Response {
    uint8_t stx;
    uint8_t seq;
    uint8_t ack;
    uint8_t etx;
} Response;

static int get_fifo(const char *fifo_path, int newflags, int openflags) {
  struct stat buf;
  int pipe_fd;

  if (stat(fifo_path, &buf) < 0) {           
    if (errno == ENOENT) {                    
      if (mkfifo(fifo_path, newflags) != 0) {
        perror("Can't create fifo");
        return -1;
      }
    } else { 
      perror("Can't stat fifo");
      return -1;
    }
  } else {                        
    if (!S_ISFIFO(buf.st_mode)) { 
      fprintf(stderr, "%s is not a pipe\n", fifo_path);
      return -1;
    }
  }

  pipe_fd = open(fifo_path, openflags);
  if (pipe_fd < 0) {
    perror("Can't open pipe");
  }

  return pipe_fd;
}

static int transmit_buf(int fd, void* buf, size_t len, int timeout_ms) {
    size_t sent = 0;
    struct timeval time = {
        .tv_sec = 0,
        .tv_usec = timeout_ms * 1000,
    };

    while (sent < len) {
        if (time.tv_usec < 0) {
            return -1;
        }

        fd_set set;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        if (select(fd + 1, NULL, &set, NULL, &time) != 0) {
            int status = write(fd, buf + sent, len - sent);
            if (status < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                continue;
            } else {
                sent += status;
            }
        } else {
            return -1;
        }
    }

    return 0;
}


static int receive_response(int fd, Response* buf, int timeout_ms) {
    size_t len = sizeof(Response);
    struct timeval time = {
        .tv_sec = 0,
        .tv_usec = timeout_ms * 1000,
    };

    while (true) {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        if (select(fd + 1, &set, NULL, NULL, &time) != 0) {
            int status = read(fd, buf, len);
            if (status < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                continue;
            }
            if (status != 0) {
                return 0;
            }
        } else {
            return -1;
        }
    }
}

static int receive_transmit_block(int fd, L2_TB_BLOCK* tb) {
    size_t len = sizeof(L2_TB_BLOCK);

    while (true) {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        if (select(fd + 1, &set, NULL, NULL, NULL) != 0) {
            int status = read(fd, tb, len);
            if (status < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                continue;
            }
            if (status != 0) {
                return 0;
            }
        } else {
            return -1;
        }
    }
}

int layer1_transmit(L2_TB_BLOCK* pkg) {
    puts("layer1_transmit");

    int pipe = get_fifo("pipe", 0644, O_CREAT | O_RDWR);
    int conf = get_fifo("conf", 0644, O_CREAT | O_RDONLY);

    int errors = 0;
    while (errors < TTR) {
        puts("transmit_buf");
        transmit_buf(pipe, pkg, sizeof(L2_TB_BLOCK), TIMEOUT_MS);

        Response res;
        puts("receive_buf");
        if (receive_response(conf, &res, TIMEOUT_MS) < 0) { 
            errors += 1;
            continue;
        }

        printf("Response:\nstx: %i\nseq: %i\nack: %i\netx: %i\n", res.stx, res.seq, res.ack, res.etx);

        if (res.seq != pkg->seq) {
            errors += 1;
            continue;
        }
        if (res.ack == 0x15) {
            errors += 1;
            continue;
        } else if (res.ack == 0x06) {
            return 0;
        } else {
            errors += 1;
            continue;
        }
    }
    return -1;
}

int layer1_receive(L2_TB_BLOCK* pkg) {
    puts("layer1_receive");

    int pipe = get_fifo("pipe", 0644, O_RDWR);
    int conf = get_fifo("conf", 0644, O_WRONLY);

    puts("receive_buf");

    receive_transmit_block(pipe, pkg); 

    Response res = {
        .stx = 0x02,
        .seq = pkg->seq,
        .ack = 0x06,
        .etx = 0x03,
    };

    uint16_t chksum = pkg->chksum;
    pkg->chksum = 0;
    if (gen_crc16((uint8_t*)pkg, sizeof(L2_TB_BLOCK)) != chksum) {
        res.ack = 0x15;
        puts("transmit_buf");
        transmit_buf(conf, &res, sizeof(Response), TIMEOUT_MS);
        return -1;
    }

    puts("transmit_buf");
    transmit_buf(conf, &res, sizeof(Response), TIMEOUT_MS);
    return 0;
}