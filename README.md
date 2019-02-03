# VASP-post-processing
Some codes/scripts I wrote for post-processing the ugly formatted VASP outputs.

## Get temperatures at all the time steps from OSZICAR produced by an MD simulation
`grep T OSZICAR | cut -d. -f1 | awk '{ print $3 }' | cat > temp.txt` 

## Get the position and force data from the OUTCAR file
* Firtst, get some line numbers from OUTCAR
`grep -n POSITION OUTCAR | cut -d: -f1 | cat > lines.txt`
* Then, compile and run `get_xf.cpp`. This code requires inputing two numbers from screen, the number of atoms and the number of lines in `lines.txt`.
