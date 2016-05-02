internal_ip_array=("10.142.0.2" "10.142.0.3")

sudo chmod 600 /local/DecSearch/distributed/utils/others/gce_key_openssh

# prompts first
for internal_ip in "${internal_ip_array[@]}"
do
    ssh -i /local/DecSearch/distributed/utils/others/gce_key_openssh zlu12@${internal_ip} 'sh' < /local/DecSearch/distributed/utils/setup_decsearch_remote.sh
done
ssh-copy-id zlu12@lanterns2.eecs.utk.edu

for internal_ip in "${internal_ip_array[@]}"
do
    ssh zlu12@${internal_ip} 'sudo apt-get update'
    ssh zlu12@${internal_ip} 'sudo apt-get -y install libopenmpi-dev openmpi-bin default-jdk'
    ssh zlu12@${internal_ip} 'sudo chown zlu12 /local'
    scp /local/DecSearch/distributed/utils/sshd_config zlu12@${internal_ip}:~/
    ssh zlu12@${internal_ip} 'sudo mv /users/zlu12/sshd_config /etc/ssh/'
done

sudo apt-get update
sudo apt-get -y install vim ssh gcc g++ build-essential cmake zlib1g-dev git automake  

cd ~ 
git clone https://github.com/dato-code/PowerGraph.git
cd ~/PowerGraph/apps
mkdir ds_dist
cd ~/PowerGraph/apps/ds_dist
ln -s ~/DecSearch/distributed/build_tree.hpp
ln -s ~/DecSearch/distributed/common.hpp
ln -s ~/DecSearch/distributed/dec_search.hpp
ln -s ~/DecSearch/distributed/ds_dist.cpp
ln -s ~/DecSearch/distributed/query_handler.hpp
ln -s ~/DecSearch/distributed/real_dist.hpp
ln -s ~/DecSearch/distributed/utils/CMakeLists.txt
cd ~/PowerGraph/src/graphlab/engine/
cp ~/DecSearch/distributed/engine/* .

cd ~/PowerGraph
./configure 
cp ~/DecSearch/distributed/utils/boost_1_53_0.tar.gz ~/PowerGraph/deps/boost/src/
cp ~/DecSearch/distributed/utils/libevent-2.0.18-stable.tar.gz ~/PowerGraph/deps/event/src/
cd ~/PowerGraph/release/apps/ds_dist/
make -j4

ln -s ~/DecSearch/distributed ~/PowerGraph/release/apps/ds_dist/src
ln -s ~/datasets ~/PowerGraph/release/apps/ds_dist/datasets
ln -s ~/DecSearch/results ~/PowerGraph/release/apps/ds_dist/results
ln -s ~/DecSearch/distributed/utils/simple_multi.sh ~/PowerGraph/release/apps/ds_dist/simple.sh
cp ~/DecSearch/distributed/utils/load.sh ~/PowerGraph/release/apps/ds_dist/load.sh
mkdir binary
mv ds_dist binary/ds_dist_test

rm ~/PowerGraph/release/apps/ds_dist/machines
for internal_ip in "${internal_ip_array[@]}"
do
    echo ${internal_ip} >> ~/PowerGraph/release/apps/ds_dist/machines
done

ln ~/PowerGraph/release/apps/ds_dist/machines ~/machines

echo "You are all set."
