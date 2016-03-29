#!/bin/sh

# prompts first

ssh-keygen
ssh-copy-id lanterns2.eecs.utk.edu

sudo chown zlu12 /local
sudo chown zlu12 /mydata

# svn co --password Xiaoyan0308 https://com1333.eecs.utk.edu:8443/svn/source/Codes/DecSearch /local/DecSearch
# svn co --password Xiaoyan0308 https://com1333.eecs.utk.edu:8443/svn/source/Codes/Scripts /local/Scripts

scp lanterns2.eecs.utk.edu:/local_scratch/Datasets/graph_datasets/large/* /mydata
scp lanterns2.eecs.utk.edu:/local_scratch/Datasets/graph_datasets/regular/* /mydata
scp -r lanterns2.eecs.utk.edu:/local_scratch/Datasets/graph_datasets/testcases /mydata

sudo apt-get update
sudo apt-get -y install vim ssh gcc g++ build-essential libopenmpi-dev openmpi-bin default-jdk cmake zlib1g-dev git automake  

cd /local
git clone https://github.com/dato-code/PowerGraph.git
cd /local/PowerGraph/apps
mkdir ds_dist
cd /local/PowerGraph/apps/ds_dist
ln -s /local/DecSearch/distributed/build_tree.hpp
ln -s /local/DecSearch/distributed/common.hpp
ln -s /local/DecSearch/distributed/dec_search.hpp
ln -s /local/DecSearch/distributed/ds_dist.cpp
ln -s /local/DecSearch/distributed/query_handler.hpp
ln -s /local/DecSearch/distributed/real_dist.hpp
ln -s /local/DecSearch/distributed/utils/CMakeLists.txt
cd /local/PowerGraph/src/graphlab/engine/
cp /local/DecSearch/distributed/engine/* .

cd /local/PowerGraph
./configure --no_jvm
cp /local/DecSearch/distributed/utils/boost_1_53_0.tar.gz /local/PowerGraph/deps/boost/src/
cp /local/DecSearch/distributed/utils/libevent-2.0.18-stable.tar.gz /local/PowerGraph/deps/event/src/
cd /local/PowerGraph/release/apps/ds_dist/
make -j8

ln -s /mydata /local/PowerGraph/release/apps/ds_dist/datasets
ln -s /local/DecSearch/results /local/PowerGraph/release/apps/ds_dist/results
ln -s /local/DecSearch/distributed/utils/simple_monster.sh /local/PowerGraph/release/apps/ds_dist/simple.sh
mkdir binary
mv ds_dist binary/ds_dist_test

hostname > /local/PowerGraph/release/apps/ds_dist/machines
sudo ln -s /local/Scripts/vimrc.local /etc/vim/vimrc.local

echo "You are all set."
