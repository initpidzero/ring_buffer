#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define BUF_SIZ 20

struct ring {
        unsigned int head; /* head will always be ahead by 1 */
        unsigned int tail;
        char buffer[BUF_SIZ];
};

struct print_buf{
        unsigned char empty;
        unsigned char value;
        unsigned char head_or_tail;
};

static int is_full(struct ring *buf)
{
        if (((buf->head > buf->tail) && (buf->head- buf->tail == BUF_SIZ - 2))
            || ((buf->head < buf->tail) && (buf->tail - buf->head == 2))) {
                printf("is full\n");
                return 1;
        } else
                return 0;
}

static int is_empty(struct ring *buf)
{
        if (buf->head == buf->tail) {
                printf("is empty\n");
                return 1;
        } else
                return 0;
}

static int rem_from_buf(struct ring *buf, char *in)
{
        if (is_empty(buf))
                return 1;

        *in = buf->buffer[buf->tail];
        buf->tail++;
        buf->tail = buf-> tail % (BUF_SIZ);
        return 0;
}

static int add_to_buf(struct ring *buf, char in)
{
        if (is_full(buf))
                return 1;
        printf("%c %d\n", in, buf->head);
        buf->buffer[buf->head] = in;
        buf->head++;
        buf->head = buf->head % (BUF_SIZ);
        return 0;
}

static void init_buf(struct ring *buf)
{
        memset(buf->buffer, 0, BUF_SIZ);
        buf->head = 0;
        buf->tail = 0;
}

static int read_buf(char *buf, struct ring *ring_buf)
{
        char *temp = strtok(NULL, " \n");
        if (!temp) {
                fprintf(stderr, "Missing number\n");
                return 1;
        }
        int num = strtol(temp, NULL, 10);
        int i;
        char in;
        for (i = 0; i < num; i++) {
                if (rem_from_buf(ring_buf, &in))
                        break;
                printf("%c\n", in);
        }
        return 0;
}

static int write_buf(char *buf, struct ring *ring_buf)
{
        char *temp;
        while (1) {
                temp = strtok(NULL, " ");
                if (temp == NULL)
                        break;
                printf("we got %s\n", temp);
                if (add_to_buf(ring_buf, temp[0]))
                       break;
        }
        return 0;
}

static void print_ring(struct ring *buf)
{
        int i = buf->tail;
        int j = 0;
        struct print_buf pb[BUF_SIZ];
        memset(pb, 0, BUF_SIZ * sizeof(struct print_buf));

        printf("|tail = %d", i);

        /*head is ahead by one */
        while (i != buf->head) {
                printf("|%d=%c", i, buf->buffer[i]);
                pb[i].value = buf->buffer[i];
                pb[i].head_or_tail = 0;
                if (i == buf->tail)
                        pb[i].head_or_tail = 'T';
                pb[i].empty = 'F';
                i++;
                i = i % (BUF_SIZ);
        }

        printf("|head = %d\n", i);
        /* head is ahead by one, so we rewind back one position to get */
        if (i != 0)
                pb[i].head_or_tail = 'H';
        else
                if(buf->tail == 0)
                        pb[i].head_or_tail = 'T';
                else
                        pb[BUF_SIZ - 1].head_or_tail = 'H';

        for (;j < BUF_SIZ; j++) {
                printf("|");
                if (pb[j].head_or_tail == 'T')
                        printf("T");
                if (pb[j].head_or_tail == 'H')
                        printf("H");
                if (pb[j].empty == 'F')
                        printf("%c", pb[j].value);
                else
                        printf("-");
        }
        printf("|\n");
}

static int ring_loop(int *exit, char *buf, struct ring *ring_buf)
{
        int com = 0;
        char *token  = NULL;

        token = strtok(buf, " ");
        if (token == NULL)
                fprintf(stderr, "strtok failed : %s\n", strerror(errno));

                if (strncmp("write", token, strlen("write"))==0) {
                        write_buf(buf, ring_buf);
                 } else if (strncmp("read", token, strlen("read"))==0) {
                         read_buf(buf, ring_buf);
                 } else if (strncmp("print", token, strlen("print"))==0) {
                         print_ring(ring_buf);
                 } else if (strncmp("exit", token, strlen("exit"))==0) {
                         *exit = 0;
                 } else  {
                         fprintf(stderr, "unknown command\n");
                 }
        return 0;
}

int main(void)
{
        int exit = 1; /* The value is changed to zero when user calls quit command */
        char prompt[] = "(ring):";
        char buffer[BUF_SIZ];
        struct ring ring_buf;
        int i;
        init_buf(&ring_buf);
        char c;
        while (exit) {
                ssize_t bytes_read;

                errno = 0;
                if (write(STDOUT_FILENO, prompt, strlen(prompt) + 1) == -1 || errno != 0)
                        fprintf(stderr, "write failed : %s\n", strerror(errno));

                bzero(buffer, BUF_SIZ);
                errno = 0;
                bytes_read = read(STDIN_FILENO, buffer, BUF_SIZ);
                if (bytes_read == -1 || errno != 0)
                        fprintf(stderr, "read failed : %s\n", strerror(errno));
                assert(bytes_read > 0);

                ring_loop(&exit, buffer, &ring_buf);
        }
        /*while (1)
              c = fgetc(stdin); 
        for (i = 0; i < 18; i++) {
                add_to_buf(&buf, 'a' + i);
        }
                add_to_buf(&buf, 'a' + i);
        for (i = 0; i < 18; i++) {
                rem_from_buf(&buf, &c);
        }
                rem_from_buf(&buf, &c);
        print_buf(&buf);
        for (i = 0; i < 18; i++) {
                add_to_buf(&buf, 'a' + i);
        }
                rem_from_buf(&buf, &c);
                add_to_buf(&buf, 'a' + i);
                rem_from_buf(&buf, &c);
                add_to_buf(&buf, 'a' + i);
        print_buf(&buf); */

        return 0;
}
