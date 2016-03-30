echo $$
echo "%CPU %MEM ARGS $(date)" > load.log
while true;
do 
    if [ -e "load.log" ]
    then
        ps -e -o pcpu,pmem,args | grep "ds_dist" | grep -v "grep" >> load.log;
    fi
    sleep 1;
done
