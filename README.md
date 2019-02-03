# VASP-post-processing
Some codes/scripts I wrote for post-processing the ugly formatted VASP outputs.

## Get temperatures at all the time steps from OSZICAR produced by an MD simulation
`grep T OSZICAR | cut -d. -f1 | awk '{ print $3 }' | cat > temp.txt` 

## Get the position and force data from the OUTCAR file
* Firtst, get some line numbers from OUTCAR
`grep -n POSITION OUTCAR | cut -d: -f1 | cat > lines.txt`
* Then, compile and run `get_xf.cpp`. This code requires inputing two numbers from the screen, the number of atoms `N` and the number of lines in `lines.txt`. Upon finished, a file named `xf.txt` will be produced. In this file, each line contains 6 numbers: x, y, z, fx, fy, fz. Each consecutive `N` lines correspond to a time step.
