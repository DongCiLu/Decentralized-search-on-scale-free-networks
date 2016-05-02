internal_ip_array=("10.142.0.2" "10.142.0.3")

sudo chmod 600 ~/DecSearch/distributed/utils/others/gce_key_openssh

ssh-keygen -t rsa -b 2048
# prompts first
for internal_ip in "${internal_ip_array[@]}"
do
    cat ~/.ssh/id_rsa.pub | ssh -i ~/DecSearch/distributed/utils/others/gce_key_openssh ${internal_ip} "cat - >> ~/.ssh/authorized_key2"
done
ssh-copy-id zlu12@lanterns2.eecs.utk.edu

exit(1)

for internal_ip in "${internal_ip_array[@]}"
do
    ssh ${internal_ip} 'sudo apt-get update'
    ssh ${internal_ip} 'sudo apt-get -y install libopenmpi-dev openmpi-bin default-jdk'
    scp /local/DecSearch/distributed/utils/sshd_config ${internal_ip}:~/
    ssh ${internal_ip} 'sudo mv /users/zlu12/sshd_config /etc/ssh/'
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
