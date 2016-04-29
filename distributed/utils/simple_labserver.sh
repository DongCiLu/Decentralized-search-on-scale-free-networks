# gn_array=("wiki" "skitter" "bio" "baidu" "Livejournal" "hollywood" "orkut" "sinaweibo" "webuk" "friendster")
gn_array=("skitter")

data_folder="datasets"
exec_folder="binary"
res_folder="results"
# testcase_folder="datasets/testcases/withreal" # use when need accuracy
testcase_folder="datasets/testcases/regular"

n_cores=2
n_exp=(100000)
n_machines=(1)
n_tree=(2)
stepy_flag=0
posfix=("bi") # if use real, dont forget to change the testcase directory

pre1="mkdir ./${res_folder}/class/"
pre2="bash -x /home/$USER/PowerGraph/scripts/mpirsync"
cmd1="mpiexec -n n_machines --hostfile /home/$USER/machines ./${exec_folder}/ds_dist_opt --graph ./datasets/graphname_wcc.txt --ncpus ${n_cores} --saveprefix graphname_class_n_machinesm_n_expq --num_tree n_tree --stepy ${stepy_flag} --num_query n_exp --input_file ./${testcase_folder}/graphname_testcases.txt"
cmd2="mv graphname_class_n_machinesm_n_expq* ./${res_folder}/class/"

$pre2
for i in "${gn_array[@]}"
do
    cm1_mod1=${cmd1//graphname/$i}
    cm2_mod1=${cmd2//graphname/$i} 

    for ((j=0; j<${#n_tree[@]}; j++));
    do
        cm1_mod2=${cm1_mod1//opt/${posfix[$j]}} 
        cm1_mod3=${cm1_mod2//n_tree/${n_tree[$j]}} 
        class="${n_tree[$j]}t_${posfix[$j]}"
        ${pre1//class/$class}
        for ((k=0; k<${#n_machines[@]}; k++));
        do
            cm1_mod4=${cm1_mod3//n_machines/${n_machines[$k]}} 
            cm2_mod2=${cm2_mod1//n_machines/${n_machines[$k]}}
            for ((l=0; l<${#n_exp[@]}; l++));
            do
                cm1_mod5=${cm1_mod4//n_exp/${n_exp[$l]}} 
                echo ${cm1_mod5//class/$class}
                # ./load.sh &
                # PIDLOAD=$!
                # echo $PIDLOAD
                ${cm1_mod5//class/$class}
                # PID=$!
                # wait $PID
                # kill $PIDLOAD
                cm2_mod3=${cm2_mod2//n_exp/${n_exp[$l]}}
                ${cm2_mod3//class/$class}
            done
        done
    done
done
