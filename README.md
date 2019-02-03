# VASP-post-processing
Some codes/scripts I wrote for post-processing the ugly formatted VASP outputs. The purpose is to extract some data buried in VASP output files and then write them to better formatted files.

## Get temperatures at all the time steps from OSZICAR produced by an MD simulation
`grep T OSZICAR | cut -d. -f1 | awk '{ print $3 }' | cat > temp.txt` 

## Get the position and force data from the OUTCAR file
* Firtst, get some line numbers from OUTCAR
`grep -n POSITION OUTCAR | cut -d: -f1 | cat > lines.txt`
* Then, compile and run `get_xf.cpp`. This code requires inputing two numbers from the screen, the number of atoms `N` and the number of lines in `lines.txt`. Upon finished, a file named `xf.txt` will be produced. In this file, each line contains 6 numbers: `x, y, z, fx, fy, fz`. Each consecutive `N` lines correspond to a time step.

## Get the thermal conductivity from the position data and pre-computed force constants using the Green-Kubo method
* First compute the force constants in some way
* Then get the file `xf.txt` using the `get_xf.cpp` code
* Then calculate the thermal conductivity using the `kappa.cpp` code
