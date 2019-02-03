# VASP-post-processing
Some codes/scripts I wrote for post-processing the ugly formatted VASP outputs.

## Get temperature from OSZICAR produced by an MD simulation:
`grep T OSZICAR | cut -d. -f1 | awk '{ print $3 }' | cat > temp.txt` 


