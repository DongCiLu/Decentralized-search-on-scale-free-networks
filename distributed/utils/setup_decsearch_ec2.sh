# internal_ip_array=("10.142.0.2" "10.142.0.3" "10.142.0.4" "10.142.0.5")

mkdir /home/ubuntu/datasets
sudo mount /dev/sdb1 /home/ubuntu/datasets
sudo chmod 600 /home/ubuntu/DecSearch/distributed/utils/others/dc_ec2.pem
sudo cp /home/ubuntu/DecSearch/distributed/utils/sshd_config /etc/ssh/sshd_config
sudo service ssh restart

ssh-keygen -t rsa -b 2048
for internal_ip in "${internal_ip_array[@]}"
do
    cat /home/ubuntu/.ssh/id_rsa.pub | ssh -i /home/ubuntu/DecSearch/distributed/utils/others/dc_ec2.pem ${internal_ip} "cat - > /home/ubuntu/.ssh/authorized_keys2"
    cat /home/ubuntu/.ssh/id_rsa | ssh ${internal_ip} "cat - > /home/ubuntu/.ssh/id_rsa"
    ssh ${internal_ip} "sudo cat /home/ubuntu/.ssh/authorized_keys2 > /home/ubuntu/.ssh/id_rsa.pub"
    ssh ${internal_ip} "sudo chmod 600 /home/ubuntu/.ssh/id_rsa"
    ssh ${internal_ip} "sudo chmod 644 /home/ubuntu/.ssh/authorized_keys2"
done

for internal_ip in "${internal_ip_array[@]}"
do
    ssh ${internal_ip} 'sudo apt-get update'
    ssh ${internal_ip} 'sudo apt-get -y install libopenmpi-dev openmpi-bin default-jdk zlib1g-dev'
done

sudo apt-get update
sudo apt-get -y install vim ssh gcc g++ build-essential cmake git automake  

cd /home/ubuntu 
git clone https://github.com/dato-code/PowerGraph.git
cd /home/ubuntu/PowerGraph/apps
mkdir ds_dist
cd /home/ubuntu/PowerGraph/apps/ds_dist
ln -s /home/ubuntu/DecSearch/distributed/build_tree.hpp
ln -s /home/ubuntu/DecSearch/distributed/common.hpp
ln -s /home/ubuntu/DecSearch/distributed/dec_search.hpp
ln -s /home/ubuntu/DecSearch/distributed/ds_dist.cpp
ln -s /home/ubuntu/DecSearch/distributed/query_handler.hpp
ln -s /home/ubuntu/DecSearch/distributed/real_dist.hpp
ln -s /home/ubuntu/DecSearch/distributed/utils/CMakeLists.txt
cd /home/ubuntu/PowerGraph/src/graphlab/engine/
cp /home/ubuntu/DecSearch/distributed/engine/* .

cd /home/ubuntu/PowerGraph
./configure 
cp /home/ubuntu/DecSearch/distributed/utils/boost_1_53_0.tar.gz /home/ubuntu/PowerGraph/deps/boost/src/
cp /home/ubuntu/DecSearch/distributed/utils/libevent-2.0.18-stable.tar.gz /home/ubuntu/PowerGraph/deps/event/src/
cd /home/ubuntu/PowerGraph/release/apps/ds_dist/
make -j4

ln -s /home/ubuntu/DecSearch/distributed /home/ubuntu/PowerGraph/release/apps/ds_dist/src
ln -s /home/ubuntu/datasets /home/ubuntu/PowerGraph/release/apps/ds_dist/datasets
mkdir /home/ubuntu/PowerGraph/release/apps/ds_dist/results
ln -s /home/ubuntu/DecSearch/distributed/utils/simple_ec2.sh /home/ubuntu/PowerGraph/release/apps/ds_dist/simple.sh
mkdir binary
mv ds_dist binary/ds_dist_test

rm /home/ubuntu/machines
for internal_ip in "${internal_ip_array[@]}"
do
    echo ${internal_ip} >> /home/ubuntu/machines
done

echo "You are all set."
