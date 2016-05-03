# internal_ip_array=("10.142.0.2" "10.142.0.3" "10.142.0.4" "10.142.0.5")
internal_ip_array=("10.142.0.2" "10.142.0.3" "10.142.0.4" "10.142.0.5" "10.142.0.6" "10.142.0.7" "10.142.0.8" "10.142.0.9" "10.142.0.10" "10.142.0.11" "10.142.0.12" "10.142.0.13" "10.142.0.14" "10.142.0.15" "10.142.0.16" "10.142.0.17" "10.142.0.18" "10.142.0.19" "10.142.0.20" "10.142.0.21")

mkdir /home/luzheng0314/datasets
sudo mount /dev/sdb1 /home/luzheng0314/datasets
ssh-keygen -t rsa -b 2048
sudo chmod 600 /home/luzheng0314/DecSearch/distributed/utils/others/gce_key_openssh

    sudo cp /home/luzheng0314/DecSearch/distributed/utils/sshd_config /etc/ssh/sshd_config
    sudo service ssh restart

for internal_ip in "${internal_ip_array[@]}"
do
    cat /home/luzheng0314/.ssh/id_rsa.pub | ssh -i /home/luzheng0314/DecSearch/distributed/utils/others/gce_key_openssh ${internal_ip} "cat - > /home/luzheng0314/.ssh/authorized_keys2"
    cat /home/luzheng0314/.ssh/id_rsa | ssh ${internal_ip} "cat - > /home/luzheng0314/.ssh/id_rsa"
    ssh ${internal_ip} "sudo cat /home/luzheng0314/.ssh/authorized_keys2 > /home/luzheng0314/.ssh/id_rsa.pub"
    ssh ${internal_ip} "sudo chmod 600 /home/luzheng0314/.ssh/id_rsa"
    ssh ${internal_ip} "sudo chmod 644 /home/luzheng0314/.ssh/authorized_keys2"
done
ssh-copy-id zlu12@lanterns2.eecs.utk.edu

for internal_ip in "${internal_ip_array[@]}"
do
    ssh ${internal_ip} 'sudo apt-get update'
    ssh ${internal_ip} 'sudo apt-get -y install libopenmpi-dev openmpi-bin default-jdk'
done

sudo apt-get update
sudo apt-get -y install vim ssh gcc g++ build-essential cmake zlib1g-dev git automake  

cd /home/luzheng0314 
git clone https://github.com/dato-code/PowerGraph.git
cd /home/luzheng0314/PowerGraph/apps
mkdir ds_dist
cd /home/luzheng0314/PowerGraph/apps/ds_dist
ln -s /home/luzheng0314/DecSearch/distributed/build_tree.hpp
ln -s /home/luzheng0314/DecSearch/distributed/common.hpp
ln -s /home/luzheng0314/DecSearch/distributed/dec_search.hpp
ln -s /home/luzheng0314/DecSearch/distributed/ds_dist.cpp
ln -s /home/luzheng0314/DecSearch/distributed/query_handler.hpp
ln -s /home/luzheng0314/DecSearch/distributed/real_dist.hpp
ln -s /home/luzheng0314/DecSearch/distributed/utils/CMakeLists.txt
cd /home/luzheng0314/PowerGraph/src/graphlab/engine/
cp /home/luzheng0314/DecSearch/distributed/engine/* .

cd /home/luzheng0314/PowerGraph
./configure 
cp /home/luzheng0314/DecSearch/distributed/utils/boost_1_53_0.tar.gz /home/luzheng0314/PowerGraph/deps/boost/src/
cp /home/luzheng0314/DecSearch/distributed/utils/libevent-2.0.18-stable.tar.gz /home/luzheng0314/PowerGraph/deps/event/src/
cd /home/luzheng0314/PowerGraph/release/apps/ds_dist/
make -j4

ln -s /home/luzheng0314/DecSearch/distributed /home/luzheng0314/PowerGraph/release/apps/ds_dist/src
ln -s /home/luzheng0314/datasets /home/luzheng0314/PowerGraph/release/apps/ds_dist/datasets
mkdir /home/luzheng0314/PowerGraph/release/apps/ds_dist/results
ln -s /home/luzheng0314/DecSearch/distributed/utils/simple_gce.sh /home/luzheng0314/PowerGraph/release/apps/ds_dist/simple.sh
cp /home/luzheng0314/DecSearch/distributed/utils/mpirsync_gce /home/luzheng0314/PowerGraph/scripts/mpirsync
mkdir binary
mv ds_dist binary/ds_dist_test

rm /home/luzheng0314/machines
for internal_ip in "${internal_ip_array[@]}"
do
    echo ${internal_ip} >> /home/luzheng0314/machines
done

echo "You are all set."
