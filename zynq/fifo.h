#define FIFO_SIZE 14
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

struct fifo{
    u8 buf_tx[FIFO_SIZE];
    u16 head_tx;
    u16 tail_tx;
    u8 tx_over;
    u8 buf_rx[FIFO_SIZE];
    u16 head_rx;
    u16 tail_rx;
    u8 rx_over;
};
u16 fifo_tx_count_data(struct fifo *pfifo){
    if ((pfifo->head_tx)>(pfifo->tail_tx))
        return (pfifo->head_tx-pfifo->tail_tx);
    return (FIFO_SIZE-pfifo->tail_tx+pfifo->head_tx);    
}
u8 fifo_tx_is_empty(struct fifo *pfifo){
    return (pfifo->head_tx==pfifo->tail_tx);
}
u8 fifo_tx_is_full(struct fifo *pfifo){
    return (fifo_tx_count_data(pfifo)==FIFO_SIZE-1);
}
void fifo_tx_erase(struct fifo *pfifo){
    pfifo->head_tx=0;
    pfifo->tail_tx=0;
}
void fifo_tx_write(struct fifo *pfifo, u8 data){   
    pfifo->buf_tx[pfifo->head_tx] = data;
    pfifo->head_tx++;
    if (pfifo->head_tx==FIFO_SIZE)
        pfifo->head_tx=0; 
}
u8 fifo_tx_read(struct fifo *pfifo){
    u8 data = pfifo->buf_tx[pfifo->tail_tx];
    pfifo->tail_tx++;
    if (pfifo->tail_tx==FIFO_SIZE)
        pfifo->tail_tx=0;    
    return data;
}
//
u16 fifo_rx_count_data(struct fifo *pfifo){
    if ((pfifo->head_rx)>(pfifo->tail_rx))
        return (pfifo->head_rx-pfifo->tail_rx);
    return (FIFO_SIZE-pfifo->tail_rx+pfifo->head_rx);    
}
u8 fifo_rx_is_empty(struct fifo *pfifo){
    return (pfifo->head_rx==pfifo->tail_rx);
}
u8 fifo_rx_is_full(struct fifo *pfifo){
    return (fifo_rx_count_data(pfifo)==FIFO_SIZE-1);
}
void fifo_rx_erase(struct fifo *pfifo){
    pfifo->head_rx=0;
    pfifo->tail_rx=0;
}
void fifo_rx_write(struct fifo *pfifo, u8 data){   
    pfifo->buf_rx[pfifo->head_rx] = data;
    pfifo->head_rx++;
    if (pfifo->head_rx==FIFO_SIZE)
        pfifo->head_rx=0; 
}
u8 fifo_rx_read(struct fifo *pfifo){
    u8 data = pfifo->buf_rx[pfifo->tail_rx];
    pfifo->tail_rx++;
    if (pfifo->tail_rx==FIFO_SIZE)
        pfifo->tail_rx=0;    
    return data;
}