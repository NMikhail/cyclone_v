#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"





int main(void){
    u8 i=0,cnt;
    char *cf;
    static struct fifo *my_fifo;
    my_fifo = malloc(sizeof(struct fifo));
    fifo_tx_erase(my_fifo);
    printf("write\n");
    while (!fifo_tx_is_full(my_fifo)){
        fifo_tx_write(my_fifo, i);
        printf("head: %d tail: %d count data: %d\n", my_fifo->head_tx, my_fifo->tail_tx,fifo_tx_count_data(my_fifo));
        i++;
    }
    printf("read\n");
    while(!fifo_tx_is_empty(my_fifo)){
        printf("data: %d\n",fifo_tx_read(my_fifo));
        printf("head: %d tail: %d count data: %d\n", my_fifo->head_tx, my_fifo->tail_tx,fifo_tx_count_data(my_fifo));
    }
    printf("write\n");
    while (!fifo_tx_is_full(my_fifo)){
        fifo_tx_write(my_fifo, i);
        printf("head: %d tail: %d count data: %d\n", my_fifo->head_tx, my_fifo->tail_tx,fifo_tx_count_data(my_fifo));
        i++;
    }
    printf("read\n");
    while(!fifo_tx_is_empty(my_fifo)){
        printf("data: %d\n",fifo_tx_read(my_fifo));
        printf("head: %d tail: %d count data: %d\n", my_fifo->head_tx, my_fifo->tail_tx,fifo_tx_count_data(my_fifo));
    }
    free(my_fifo);
    return 0;
}