echo "%CPU %MEM ARGS $(date)" > load.log
while [ -e "load.log" ];
do 
    ps -e -o pcpu,pmem,args | grep "ds_dist" | grep -v "grep\|load.sh\|mpiexec" >> load.log;
    sleep 0.1;
done
