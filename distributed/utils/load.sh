echo "%CPU %MEM ARGS $(date)" > results/load.log
while true;
do 
    ps -e -o pcpu,pmem,args | grep "ds_dist" | grep -v "grep" >> results/load.log;
    sleep 0.1;
done
