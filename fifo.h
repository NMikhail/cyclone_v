
#define FIFO(size) \
struct fifo{\
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
