#include <stdio.h>
#include <stdlib.h>

#define FIFO(size) \
struct dfd{\
    char buf[size];\
    int head;\
    int tail;\
    }

#define FIFO_SIZE(fifo)         (sizeof(fifo->buf)/sizeof(fifo->buf[0]))
#define FIFO_NUM_DATA(fifo)     (fifo->head-fifo->tail)
#define FIFO_IS_FULL(fifo)      (FIFO_NUM_DATA(fifo)==FIFO_SIZE(fifo))
#define FIFO_IS_EMPTY(fifo)     (fifo->head==fifo->tail)
#define FIFO_FREE_SPACE(fifo)   (FIFO_SIZE(fifo)-FIFO_NUM_DATA(fifo))
#define FIFO_FLUSH(fifo)        \
{\
    fifo->head=0;\
    fifo->tail=0;\
}
#define FIFO_WR(fifo, data)     \
{\
    fifo->buf[fifo->head & (FIFO_SIZE(fifo)-1)]=data;\
    fifo->head++;\
} 
#define FIFO_RD(fifo, data)           \
{\
    data=fifo->buf[fifo->tail & (FIFO_SIZE(fifo)-1)];\
    fifo->tail++;\
}

static FIFO(64) *my_fifo;  

int main(void){

    my_fifo = malloc(sizeof(*my_fifo));
    unsigned char d,b,i = 0,cnt = 0;
    while(!FIFO_IS_FULL(my_fifo)){            
        FIFO_WR(my_fifo, i);
        i++;
    }
    for (cnt=0;cnt<32;cnt++){
        FIFO_RD(my_fifo,d);
        printf("FIFO data: %d\n",d);
    }
    scanf("%c",&b);
    while(!FIFO_IS_FULL(my_fifo)){            
        FIFO_WR(my_fifo, i);
        i++;
    }

    while(!FIFO_IS_EMPTY(my_fifo)){
        FIFO_RD(my_fifo,d)
        printf("FIFO data: %d\n",d);
    }
    printf("FIFO is FULL count data: %d\n",i);

    free(my_fifo);
    return 0;
}
