# Decentralized search on power-law graphs.
Decentralized search based on landmarks for fast shortest path approximation in large-scale power-law graphs. 
See more details in our paper:
Lu, Zheng, Yunhe Feng, and Qing Cao. "Decentralized Search for Shortest Path Approximation in Large-scale Complex Networks." 2017 IEEE International Conference on Cloud Computing Technology and Science (CloudCom). IEEE, 2017.
Implemented on Powergraph, a distributed graph processing platform.


## System Overview:
The algorithm contains two phases, an offline preprocessing phase which builds landmark indices and an online query phase which performs decentralized search on the preprocessed indices.

<img src="/example/system_overview.png" width="900px"/>  

## Generate Indices:
Indices used in our system consist of several landmarks and a shortest path tree from each landmark. It is later used by decentralized search as distance estimates between arbitrary vertex pairs. To make the search works more efficiently, we want to construct the shortest path tree in a way that the distance estimates for average case is more accurate. Our idea is to index shortest pathes with higher "path degree". 

### The intuition:

<img src="/example/landmark_intuitive.png" width="450px"/>

### Index construction based on "path degree":

<img src="/example/index_construction.png" width="900px"/> 

## Search Algorithm
When performing decentralized search on landmark based indices, the search at each step only examine its neighbor and make the decision on distance estimates to the target. The search terminates once it reaches any vertex on the indexed shortest path to the target vertex.

<img src="/example/search.png" width="450px"/>

## Performance
### Tested on datasets

<img src="/example/dataset.png" width="450px"/> 

### Accuracy

<img src="/example/accuracy.png" width="900px"/> 

### Throughput

<img src="/example/throughput.png" width="900px"/> 
