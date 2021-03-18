/* This file is under GPLv2
 * This is a simple implementation of a ring/circular buffer
 * There is a command driven loop, which takes input commands and can print
 * buffer with print command
 * Author: Anuz me@anuz.me */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

/* House keeping buffer*/
#define BUF_SIZ 20

/* this is the size we assign to the ring buffer */
static unsigned long buf_siz = 0;

/* our ring, it keep tail, head pointers and buffer is the actual buffer
 * and the size of the buffer, it can store (size - 1) bytes.
 * we are using 1 byte for housekeeping purposes */
struct ring {
        char *head; /* head will always be ahead by 1 */
        char *tail;
        char *buffer;
        unsigned int size;
};

/* Checks if ring buffer is full
 * in: buf - ring buffer
 * return: 1 if it is full, 0 if it is not full */
static int is_full(struct ring *buf)
{
        /* ring is full if the abs(head - tail) == 1, we leave one space between
         * head and tail when it is full, to distinguish between empty condition. */
        if (((buf->head > buf->tail) && (buf->head - buf->tail == buf->size - 1))
            || ((buf->head < buf->tail) && (buf->tail - buf->head == 1))) {
                printf("is full\n");
                return 1;
        } else
                return 0;
}

/* Checks if ring buffer is empty
 * in: buf - ring buffer
 * return: 1 if it is emtpy, 0 if it is not empty */
static int is_empty(struct ring *buf)
{
        if (buf->head == buf->tail) {
                printf("is empty\n");
                return 1;
        } else
                return 0;
}

/* Consumer function one byte at a time
 * in: buf - ring buffer to consume from
 * out: in - put the byte here.
 * return: 1 if empty, 0 if everything is normal */
static int rem_from_buf(struct ring *buf, char *in)
{
        if (is_empty(buf))
                return 1;

        *in = *(buf->tail);
        *(buf->tail) = 0; /* this is not really required, but for printing, we are using 0 as empty lot */
        buf->tail++;
        /* Once we reach the end of the buffer, we go all the way around */
        if(buf->tail == buf->buffer + buf->size)
                buf->tail = buf->buffer;
        return 0;
}

/* Add one byte to the ring buffer
 * in: in - byte to be added to the buffer
 * out: buf - ring buffer to add this byte to
 * return: 1 if empty, 0 if everything is normal */
static int add_to_buf(struct ring *buf, char in)
{
        if (is_full(buf))
                return 1;
        printf("%c %p\n", in, buf->head);
        *(buf->head) = in;
        /* Note that we are keeping our head ahead by one byte */
        buf->head++;
        /* Once we reach the end of the buffer, we go all the way around */
        if(buf->head == buf->buffer + buf->size)
                buf->head = buf->buffer;
        return 0;
}

/* Ring buffer initialisation
 * in: buf - ring buffer to initialise
 * return: none. We should probably return error in case malloc fails */
static void init_buf(struct ring *buf)
{
        buf->size = buf_siz;
        buf->buffer = malloc(buf->size);
        if (!buf->buffer || errno) {
                fprintf(stderr, "malloc failed: %s\n", strerror(errno));
                return;
        }

        memset(buf->buffer, 0, buf->size);
        buf->head = buf->buffer;
        buf->tail = buf->buffer;
}

/* Read user input for buffer size
 * in: buf - user buffer
 * out: buf_siz - file scope extern variable for size
 * return: 1 if strtok or strtol fails, 0 if everything is normal */
static int read_size(char *buf, struct ring *ring_buf)
{
        char *temp = strtok(NULL, " \n");
        if (!temp) {
                fprintf(stderr, "Missing number\n");
                return 1;
        }
        long num = strtol(temp, NULL, 10);
        if (errno) {
                fprintf(stderr, "strtol failed : %s\n", strerror(errno));
                return 1;
        }
        else
                buf_siz = num;
        return 0;
}


/* Read user input for number of bytes to be removed buffer
 * in: buf - user buffer
 * out: ring_buf - ring buffer
 * return: 1 if strtok or strtol fails, 0 if everything is normal */
static int read_buf(char *buf, struct ring *ring_buf)
{
        char *temp = strtok(NULL, " \n");
        if (!temp) {
                fprintf(stderr, "Missing number\n");
                return 1;
        }
        int num = strtol(temp, NULL, 10);
        if (errno) {
                fprintf(stderr, "strtol failed : %s\n", strerror(errno));
                return 1;
        }
        int i;
        char in;
        for (i = 0; i < num; i++) {
                if (rem_from_buf(ring_buf, &in))
                        break;
                printf("%c\n", in);
        }
        return 0;
}

/* Read user input for bytes to be added to buffer
 * in: buf - user buffer
 * out: ring_buf - ring buffer
 * return:  0, if everything is normal */
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

/* A print helper function
 * in: buf - ring buffer
 * return: nothing */
static void print_ring(struct ring *buf)
{
        char *temp = buf->buffer;
        for (;temp < (buf->buffer + buf->size); temp++) {
                printf("|");
                if (temp == buf->head)
                        printf("H");
                if (temp == buf->tail)
                        printf("T");
                if (*temp == 0)
                        printf("-");
                else
                        printf("%c", *temp);
        }
        printf("|\n");

}

/* Use input Loop for interacting with user
 * in: buf- user buffer, ring_buf - ring buffer
 * out: exit: 1 to exit from loop
 * return 0: loop shouldn't ordinarily fail. */
static int ring_loop(int *exit, char *buf, struct ring *ring_buf)
{
        char *token  = NULL;

        token = strtok(buf, " ");
        if (token == NULL)
                fprintf(stderr, "strtok failed : %s\n", strerror(errno));

        if (strncmp("size", token, strlen("size"))==0) {
                read_size(buf, ring_buf);
                init_buf(ring_buf);
        } else if (strncmp("write", token, strlen("write"))==0) {
                if (buf_siz)
                        write_buf(buf, ring_buf);
                else
                        fprintf(stderr, "buffer is zero size\n");
        } else if (strncmp("read", token, strlen("read"))==0) {
                if (buf_siz)
                        read_buf(buf, ring_buf);
                else
                        fprintf(stderr, "buffer is zero size\n");
        } else if (strncmp("print", token, strlen("print"))==0) {
                if (buf_siz)
                        print_ring(ring_buf);
                else
                        fprintf(stderr, "buffer is zero size\n");
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
        while (exit) {
                ssize_t bytes_read;

                errno = 0;
                if (write(STDOUT_FILENO, prompt, strlen(prompt) + 1) == -1
                    || errno != 0)
                        fprintf(stderr, "write failed : %s\n", strerror(errno));

                bzero(buffer, BUF_SIZ);
                errno = 0;
                bytes_read = read(STDIN_FILENO, buffer, BUF_SIZ);
                if (bytes_read == -1 || errno != 0)
                        fprintf(stderr, "read failed : %s\n", strerror(errno));
                assert(bytes_read > 0);

                ring_loop(&exit, buffer, &ring_buf);
        }

        /* Freedom */
        if (ring_buf.buffer) free(ring_buf.buffer);

        return 0;
 }
