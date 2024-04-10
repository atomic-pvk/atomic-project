void receive(struct r *);                              /* receive packet */
void packet(struct p *, struct r *);                   /* process packet */
void clock_filter(struct p *, double, double, double); /* filter */
double root_dist(struct p *);                          /* calculate root distance */
int fit(struct p *);                                   /* determine fitness of server */
void clear(struct p *, int);                           /* clear association */
int access(struct r *);