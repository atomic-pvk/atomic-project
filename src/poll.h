void poll(struct p *);                /* poll process */
void poll_update(struct p *, int);    /* update the poll interval */
void peer_xmit(struct p *);           /* transmit a packet */
void fast_xmit(struct r *, int, int); /* transmit a reply packet */