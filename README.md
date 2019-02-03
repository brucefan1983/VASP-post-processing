# VASP-post-processing
Some codes/scripts I wrote for post-processing the ugly formatted VASP outputs.

## Some simple bash commands for ectracting data:
* Get temperatures at all the time steps from OSZICAR produced by an MD simulation:
`grep T OSZICAR | cut -d. -f1 | awk '{ print $3 }' | cat > temp.txt` 
* Get some line numbers from OUTCAR, which can be used to extract the position and force data later:
`grep -n POSITION OUTCAR | cut -d: -f1 | cat > lines.txt`


