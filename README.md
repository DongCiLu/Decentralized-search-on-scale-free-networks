# Decentralized search on power-law graphs.

Decentralized search based on landmarks for fast shortest path approximation in large-scale power-law graphs.
Implemented on Powergraph, a distributed graph processing platform.


System Overview:
The algorithm contains two phases, an offline preprocessing phase which builds landmark indices and an online query phase which performs decentralized search on the preprocessed indices.

![alt text](./example/system_overview.png?raw=true "System Overview")     


We use our dataset to train both unconditional GAN and conditional GAN, based on TFGAN library and get some preliminary results:

1. Unconditional GAN:

![alt text](./examples/unconditional_gan.png?raw=true "Results for unconditional GAN")

2. Conditional GAN (condition on time frame to show the cell growth):

![alt text](./examples/conditional_gan.png?raw=true "Results for conditional GAN")
