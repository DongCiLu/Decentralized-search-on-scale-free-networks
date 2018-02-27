# Decentralized search on power-law graphs.

Decentralized search based on landmarks for fast shortest path approximation in large-scale power-law graphs.
Implemented on Powergraph, a distributed graph processing platform.


Raw and denoised image of celegans slice:

![alt text](./examples/raw_image.jpg?raw=true "Raw Image of celegans slice")     ![alt text](./examples/denoised_image.jpg?raw=true "Denoised Image of celegans slice")


We use our dataset to train both unconditional GAN and conditional GAN, based on TFGAN library and get some preliminary results:

1. Unconditional GAN:

![alt text](./examples/unconditional_gan.png?raw=true "Results for unconditional GAN")

2. Conditional GAN (condition on time frame to show the cell growth):

![alt text](./examples/conditional_gan.png?raw=true "Results for conditional GAN")
