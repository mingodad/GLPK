#!/bin/bash

#expect an ordered list of backup files in bkp-list.txt
#for an example dowload indent backup files with "wget -nc -i filelist.txt"

bkp_ext=".tar.gz"
git_folder=src

if [[ ! -e $git_folder ]]
then
    mkdir $git_folder
    git init $git_folder
    cd $git_folder
    git config diff.renames copies
    cd ..
fi

#bkp_list=(`cat bkp-list-dws.txt`)
bkp_list=(`cat bkp-list.txt`)
bkp_list_len=${#bkp_list[*]}
bkp_list_last_idx=$(( ${bkp_list_len} - 1 ))
echo $bkp_list_len $bkp_list_last_idx

for (( i=0; i < $bkp_list_len; ++i))
do
	#echo $i ${bkp_list[$i]}

	bkp_folder0=${bkp_list[$i]}
	echo $bkp_folder0
	#lp_solve_5.0
	bkp_folder=${bkp_folder0:0:12}
	echo $bkp_folder
	tar xzf $bkp_folder0$bkp_ext
	commit_date=`date -r $bkp_folder0$bkp_ext`
	cd $bkp_folder
	ln -s ../$git_folder/.git .git
	git add -u .
	git add .
	git commit --date="$commit_date" -m "backup $bkp_folder0"
	git branch ${bkp_folder0:5:12}
	cd ..

	if [[ $i != $bkp_list_last_idx ]]
	then
		#echo $i $bkp_list_last_idx
		rm -f -r $bkp_folder
	fi
done
