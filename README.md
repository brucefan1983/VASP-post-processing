# VASP-post-processing
Some codes/scripts I wrote for post-processing the ugly formatted VASP outputs.

## Get temperatures at all the time steps from OSZICAR produced by an MD simulation
`grep T OSZICAR | cut -d. -f1 | awk '{ print $3 }' | cat > temp.txt` 

## Get the position and force data from the OUTCAR file
* Firtst, get some line numbers from OUTCAR
`grep -n POSITION OUTCAR | cut -d: -f1 | cat > lines.txt`
* Then, compile and run `get_xf.cpp`. This code requires inputing two numbers from the screen, the number of atoms `N` and the number of lines (which is the number of MD steps finished) in `lines.txt`. Upon finished, a file named `xf.txt` will be produced. In this file, each line contains 6 numbers: `x, y, z, fx, fy, fz`. Each consecutive `N` lines correspond to a time step.

## Get the thermal conductivity from the position data and pre-computed force constants using the Green-Kubo method
* First compute the force constants in some way
* Then get the file `xf.txt` using the `get_xf.cpp` code
* Then calculate the thermal conductivity using the `kappa.cpp` code









VASP 学习笔记
Zheyong Fan (brucenju@gmail.com)

       这个周末学习了一点VASP计算，在此做个总结，避免忘记，同时也与读者分享。我学习VASP的主要动机是想拟合一些经验势函数和紧束缚模型的参数，分别用于大尺度分子动力学模拟和量子输运模拟。我目前没有时间自己写一个DFT程序，只能先用一个已有的。很多相关文献采用了VASP程序，我们组也买了它，所以学习这个。总的感觉是VASP很强大，但对用户不友好。这就是网络上有各种各样的VASP前处理和后处理的“工具”的原因。



1. 学习资料

（1）官网的手册和其它资料 https://cms.mpi.univie.ac.at/wiki/index.php/The_VASP_Manual

（2）大师兄的教程 https://www.bigbrosci.com/

（3）谷歌



2. 程序的购买和安装：

我们课题组已经购买，且在集群安装好5.4.4 版本。有以下可执行文件：

(1) vasp_std: 标准的版本;

(2) vasp_gam: 如果只用一个Gamma点的话，用这个版本较快;

(3) vasp_ncl: 适用于需要non-collinear自旋的计算;



3. 运行VASP（不同的课题组情况可能不一样）


（1）在集群运行，首先要加载：     

module load vasp/5.4.4
（2）然后在当前工作的文件夹准备至少四个文件，分别是INCAR、POSCAR、POTCAR、KPOINTS。

（3）然后在当前工作的文件夹运行可执行文件即可。我们集群要求这样做：    

srun vasp_std


4. 几个必须准备的输入文件的简介

（1）INCAR是用来控制整个程序的执行流程的。该文件由若干形如“关键字=参数”的语句构成。一般一个语句写一行即可，但也可以在一行写多个语句，用分号隔开。语句的次序不重要。允许出现空行和由“#”打头的注释。一共有大概200个关键字，但大都（全部？不是，比如IBRION=0时POTIM就没有默认值）有默认值。

（2）KPOINTS用来指定K点。INCAR中可对简单的情况指定K点，但更一般的情况还是需要用KPOINTS。

（3）POSCAR用来指定所研究的模型，包括周期盒子、原子组成、原子坐标等。

（4）POTCAR是赝势文件。所有的赝势文件都在集群的某个文件夹，一共几百兆。我已经将它们拷贝到我的用户目录下的某个地方。



5. POSCAR的准备

 该文件包含周期盒子、原子组成、原子坐标等信息。对于分子动力学模拟，也可以额外地包含初始速度等数据。该文件的格式为

 

第1行：注释。

第2行：标度因子。后面的盒子长度和原子坐标都会乘以这个因子。

第3-5行：周期盒子矩阵（和LAMMPS的约定差一个转置）。

第6行：模型中各个元素的符号。

第7行：与元素符号对应的周期盒子中的原子数目。

第8行：可以写S也可以没有这一行。写S代表selective dynamics，即要固定一些自由度。

第9行：可以写C或者D。前者代表后面的原子坐标用直角坐标表示，后者代表后面的原子坐标会乘以盒子原包矢量，即用所谓的分数坐标表示。

接下来：一行一行地写每个原子的坐标。原子类型的次序应该和第6、7行的信息一致。

接下来：可以写一些额外的数据，但我目前没有用到过。

 

瞎想：为什么用D代表分数坐标？D是指Direct，本意是指实空间（直接的空间；非倒空间）。因为在讨论实空间和倒空间时都会有原包的概念，所以无论是实空间还是倒空间的坐标都可以用分数表示。所以D要理解为实空间的分数坐标方案。有点绕？反正我觉得VASP的各种命名很晦涩，应该是早期用Fortran-77的缘故，导致各种命名短小而难懂。



6. POTCAR的准备

如果计算的模型中只有一种元素，那就从赝势文件夹拷贝一个合适的赝势文件到当前的工作文件夹即可。如果模型中有多种元素，就需要将这几种元素的赝势文件合并成一个新的赝势文件，次序必须与POSCAR中指定的元素次序一致。每个元素的POTCAR 都包含默认的截断能的最大值 (ENMAX) 与最小值 (ENMIN)。如果不在INCAR中指定ENCUT，那么将默认使用所有元素的POTCAR中ENMAX值的最大值。如果INCAR中指定了ENCUT，那就不会再用默认的了。

 

7. KPOINTS的准备


该文件有多种允许的格式。我目前只用一种最简单的，就是让程序自动生成K点。这种情况下还有几种可能，而我只用其中的一种。我目前用的格式如下：



第1行：注释。

第2行：写0。

第3行：写G或者M。前者代表产生的K点以Gamma点为中心，后者代表用Monkhorst-Pack的方法。

第4行：写三个正整数 N1 N2 N3，代表K空间三个基矢方向的K点数目。所以一共有N1*N2*N3个K点。

第5行：写三个整数 M1 M2 M3，代表K点的整体偏移量。一般写0 0 0 即可。



更复杂的以后再慢慢研究。



8. INCAR中的关键字

这个恐怕是学VASP最重要的部分了。我目前只学了很少（不到20%）的关键字。下面就把我能理解的关键字介绍一下。介绍的次序就比较随意了。紫色的关键字是我比较关注的（主要做分子动力学模拟）。



刚刚得到一个教训：INCAR中的关键字一定要仔细核对。如果写错了关键字，VASP不会报错（也许哪个地方有警告），只会忽略。我刚刚就把NCORE写成了NCOR。




关键字	可选值	默认值	用途与用法
NCORE	整数	1	用来同时计算一个轨道的CPU核心数。在用多核（比如32核）并行计算时，建议将该参数设置为一个计算节点的CPU核心数（比如4或8），这样一般既可提速，也会减小单核的内存压力。
SYSTEM	
字符串

unknown system	随便写个字符串描述此次计算即可；该关键字可不用，所以我也懒得写这个关键字。
NWRITE	0|1|2|3|4	2	控制在OUTCAR中写入的数据量。数字越大，写的越啰嗦。做分子动力学模拟，最好用0。但是如果要输出每一步的原子受力就需要用1了。
ISTART	0|1|2|3	如果WAVECAR输出文件存在，则为1；否则为0	0指从头算起；其它指接着以前的算，不同的数字代表不同的续算方式。我暂时懒得去理会这些细节，直接从头算起，所以不写这个关键字。
ICHARG	0|1|2|4	如果ISTART为0，则为2；否则为0	控制初始电子密度的产生方式。当ISTART=0时，ICHARG=2，指的是根据各个原子的电荷密度叠加来确定初始电子密度。所以暂时可以不用写这个关键字。
ISPIN	1|2	1	1代表不考虑自旋极化；2代表考虑。不过都是在自旋共线的情况下。不研究磁性体系，就不用写这个关键字。
MAGMOM	

用法有点复杂。我暂时不研究磁性体系，就不用写这个关键字了。
ENCUT	实数（单位是电子伏特）	POTCAR中的最大ENMAX值	指定平面波基组的截断能，单位是电子伏特。一般情况下用默认的即可。特殊情况需要增大ENCUT值。重要的是在一组相关的计算中用同一个值。
EDIFF	实数（单位是电子伏特）	1.0e-4（电子伏特）	电子自洽计算部分的收敛判据。当电子自洽循环中相邻两步的总自由能和能带能的差值都小于这个值时，就成功退出循环。一般就用默认的好了，但手册建议做分子动力学模拟时设置为1.0e-5。
EDIFFG	实数（单位是电子伏特）	EDIFF*10	离子弛豫部分的收敛判据。当离子弛豫循环中相邻两步的总自由能的差值小于这个值时，就成功退出弛豫循环。如果EDIFFG取值为负，则表示相邻两步中各个离子的受力之差都小于|EDIFFG|时退出弛豫循环。对于分子动力学模拟，这个参数会被忽略。总之，一般用默认的就好了。
PREC	Low|Medium|High|Normal|Single|Accurate	Normal	懒得去研究这个，直接用默认的Normal即可。不建议用Low，因为此时ENCUT的默认值是POTCAR中ENMIN的最大值，而不是ENMAX的最大值。VASP搞这么多选择干嘛？
ALGO	太多！	Normal	用来设置电子自洽算法等。N（普通）代表Davidson法；V（非常快）代表RMM-DIIS法；F（快）代表以上两种的某种折衷方案。其它的我暂时懒得深究。默认的应该最安全，但跑大体系的分子动力学时，建议用V。
LREAL	T|F|O|A	F	决定是否在实空间计算投影算符。F就是不在实空间；其它的都是在，只不过有些区别（暂时搞不清楚）。一般就用默认的，但手册建议原子数大于20时（例如跑分子动力学时）用A。但是做其它关于能量的计算，最好用默认的。这个需要亲自测试。
NELM	整数	60	意思就是如果电子自洽循环步数不能超过这个数。手册解释的是，如果超过这个步数还没有收敛，那就要检查其它参数了。好吧，用默认的60即可。如果是我写程序，就不会搞这个参数。
NELMIN	整数	2	电子自洽计算的最少步数。手册建议在跑分子动力学模拟时增大到4或者8，但我不知道为什么。怕出现虚假的收敛？
NSW	整数	0	离子弛豫的步数。这个在做结构优化的时候要设置。如果是跑分子动力学，那就是跑的步数。
IBRION	-1|0|1|2|3|5|6|7|8|44	NSW=0时取-1，其它情况取0	决定离子运动方式的参数。-1指啥都不做。0指分子动力学；其它的对应各种结构优化算法。1指准牛顿法。2指共轭梯度法。3指加阻尼的分子动力学方法。5-8是和声子计算有关的，我暂不研究。44是改进的Dimer法，我不想去研究。总结一下：跑分子动力学肯定要用0；做结构优化应该用1-3都可以；不考虑离子运动时就用默认的。
ISIF	0|1|2|3|4|5|6|7	IBRION=0（即做分子动力学时）取0，其它情况取2	决定算不算应力以及优化哪些离子和盒子自由度。0代表不算应力；1代表只算三个方向的平均压强；其它代表算应力张量。1-4优化离子坐标；5-7不优化离子坐标。1-2既不优化盒子大小也不优化盒子形状；3和6与1-2相反；4-5优化盒子形状但不优化盒子大小；7和4-5相反。根据以上规则来理解默认值：做分子动力学时默认不计算应力也不改变盒子（即不控压）；其它情况下计算应力张量但不改变盒子（只优化离子位置）。重要：优化盒子时ENCUT一般需要增大。手册建议增大30%，即对应PREC=High的默认值。
ISYM	-1|0|1|2|3	如果用超软赝势，则取1；如果LHFCALC设置为真，则用3；其它情况用2。	决定如何应用对称性。-1代表不用任何对称性；0代表只用psi(-k)=psi(k)*的对称性；1-3代表利用空间对称性等，只是具体做法稍有差异。如果跑分子动力学，建议用0；其它就用默认的好了。
POTIM	实数（单位取决于该参数的取值）	IBRION=0时无默认值；IBRION=1-3时取0.5；IBRION=5时取0.015埃。	如果跑分子动力学，这个参数就是积分步长（单位是fs）；如果结构优化，这个参数就是一个标度因子（无量纲）；如果是声子计算，这个参数就是位移（单位是埃）。
TEBEG	实数（单位是开尔文）	0	分子动力学模拟的初温，单位是开尔文。这个就最好不要用默认值了。
TEEND	实数（单位是开尔文）	TEBEG	分子动力学模拟的末温，单位是开尔文。根据手册，这个值只有在SMASS=-1时才会用到。这个是很容易理解的。
SMASS	-3|-2|-1|非负实数	-3	分子动力学模拟中的控温参数。-3表示用NVE系综。-2表示什么我没看懂。-1表示将温度从初温到末温线性地增加。非负实数表示用Nose控温法（应该等价于Nose-Hoover）。手册对于此时该参数选取的建议过于迂腐，而且没有指出这个参数到底是Nose热浴的哪个参数，连单位是什么都没有说。根据手册描述，该参数取0对应于40个步长，我猜测指的是Nose热浴中的时间参数等于40个步长。这个值是比较合理的，那就取 SMASS=0好了。从VASP源代码可以看出，这个参数的量纲为 [能量*时间平方]。这无疑增加了设置该参数的难度。好的做法是直接用时间作为输入参数。
ISMEAR	-5|-4|-3|-2|-1|0|正整数	1	决定电子费米分布函数的展宽方式。0是高斯展宽法；-1是费米展宽法；-2和-3基本看不懂；-4和-5是四面体法；正整数N指N阶的Methfessel-Paxton法。比较复杂。下面是从混乱的手册描述中总结的几个建议：（1）金属：用Methfessel-Paxton法（2）半导体和绝缘体（K点较少时）：用0；（3）半导体和绝缘体（K点较多时）：用-5。如果不知道体系是金属还是什么呢？
SIGMA	实数（单位是电子伏特）	0.2（电子伏特）	决定上述展宽的宽度，单位是电子伏特。建议：金属用默认值；半导体和绝缘体用0.05电子伏特。检验标准之一是OUTCAR中T*S那一项能量除以原子数小于一毫电子伏特。
MAXMIX	整数	-45	如果跑分子动力学，最好设置为第一个离子步中电子步数的三倍，对提升速度有用（我暂时的测试的结果是没什么用）。手册的例子用的40。
LCHARG	T|F	T	若为T，则输出电荷密度。如果跑分子动力学，最好设置为F，否则数据量太大。
LWAVE	T|F	T	若为T，则输出波函数。如果跑分子动力学，最好设置为F，否则数据量太大。
NBLOCK	整数	1	每隔这么多步，算一次对关联函数和态密度，输出一次原子坐标。如果跑长时间的分子动力学，最好设置大一点，否则XDATCAR文件太大。但是我都不需要XDATCAR文件了，因为我要的数据全部都在OUTCAR中。
KBLOCK	整数	NSW	每隔KBLOCK*NBLOCK步，输出一次平均的态密度（DOSCAR文件）和对关联函数（PCDAT文件）。疑问：如果NBLOCK*KBLOCK>NSW，那么是不是不会输出平均的态密度和对关联函数了？我测试的结果是：依然输出，而且似乎是每NBLOCK步输出一次。看来关于这一点手册没有讲清楚。所以，如果只想输出一次，就要确保NBOCK*KBLOCK=NSW。
ADDGRID	F|T	F	用T可能会更好地优化结构。




9. 输出文件简介

目前对我来说，重要的输出文件有：

（1）OSZICAR(为什么叫这个名字？大师兄群有人说是来自德语Oszillation，震荡的意思）

该文件包含关于电子和离子收敛情况的数据。文件不大，值得保存。

首先是电子自洽计算的收敛情况。每一步会打印如下信息

自洽算法	当前电子步	能量	总能量差	能带能差	求H*psi的次数	波函数误差	电荷密度差
然后再进行一次离子操作后打印一行信息。如果是结构优化，打印的是

当前离子步	总自由能	SIGMA=0的自由能	自由能之差	等等
如果是跑分子动力学，打印的是

当前步数	温度	系统和热浴的自由能之和	系统自由能	SIGMA=0的系统自由能	系统动能	热浴势能	热浴动能



（2）CONTCAR（CONT应该是continue的意思）。格式和POSCAR文件一致。如果是跑分子动力学，这个文件相当于续跑文件。

（3）OUTCAR（OUT就是输出的意思）这个文件包含很多数据，例如每一个离子步的原子坐标和受力（当NWRITE=0时，只有首末两步的坐标和力；当NWRITE=1时，才有每一步的）。这个文件多读读，就会懂得越来越多。

（4）关于速度数据。CONTCAR中只保留最后一步的速度数据。如果要通过速度计算扩散系数和声子态密度，该怎么办呢？我能想到的一个办法是通过对坐标求导得到速度。

（5）关于XDATCAR文件。这个文件相当于轨迹文件。但是，当NWRITE=1时，OUTCAR文件中已经有了每一步的坐标（还有对应的力）。OUTCAR中的坐标是直角坐标系的，XDATCAR中的坐标是分数坐标系的，但它们是等价的。因为我用NWRITE=1，所以我不需要XDATCAR文件中的数据了。这样我就可以专注于处理OUTCAR文件。



10. 练习写一个跑分子动力学的输入文件

准备做一个液态硅的分子动力学模拟。POSCAR、KPOINTS、POTCAR的准备都很容易（我用2*2*2*8=64个硅原子的体系；只用Gamma点; 根据文献 [1]是够了）。Here is a Matlab code to create the POSCAR file (create_poscar.m). 下面是INCAR的内容：




# Computer related
NCORE=4         # Optimal for running with e.g. 32 CPU cores
# Electron part
ALGO=V         # Use the (usually faster) RMM-DIIS algorithm for MD
LREAL=A        # Evaluate the projection operator in real space
ISYM=0         # Do not use any space symmetry
NELMIN=4    # Use at least 4 steps for the elctron part
EDIFF=1E-5   # Make the electron part more accurate
ISMEAR=0     # A very general broadening method
SIGMA=0.05  # A small value (to be safe)
# MD part
NWRITE=1      # I need the positions and forces!
LCHARG=F       # I don't need the charge density
LWAVE=F        # I don't need the wave function
SMASS=0.95     # Use the Nose thermostat (try and error)
TEBEG=1800    # Target temperature (larger than the melting point 1680 K)
POTIM=2        # A time step of 2 fs should be safe
NSW=20000   # Number of steps (Can be finished within a couple of hours)


11. 一些关于计算速度的测试结果

（0）用vasp_gam比用vasp_std要快不少，毕竟前者是专门为做Gamma点计算编译的。

（1）我们集群每个节点有16个核心，我用32个核心并行计算。测试NCORE=1、2、4、8、16的计算速度，发现NCORE 增加时，速度先增大后减小，在NCORE=4的时候达到最大。以后就用4了。

（2）ALGO的选取确实影响计算速度。V比F快，F比N快。

（3）用LREAL=A确实比默认的LREAL=F快（快25%)。

（4）MAXMIX取40、60、以及默认值，对计算速度没什么影响。在确定该参数能有助于提速之前不用它了。

（5）NELMIN=4比NELMIN=8快得多。OSZICAR显示，除了开头几个离子步，后面的离子步中电子步数都是NELMIN。问题：跑分子动力学时电子步很容易收敛吗？为什么呢？



12. 从 OSZICAR 和 OUTCAR获得数据

(1) 下面是一条可以从 OSZICAR获得温度数据的linux命令:  

grep T OSZICAR | cut -d. -f1 | awk '{ print $3 }' | cat > temp.txt
(2) 为了从OUTCAR获得坐标和力，我首先从该文件获得一些特殊的行指标：

grep -n POSITION OUTCAR | cut -d: -f1 | cat > lines.txt
根据这些行指标，我再写一个C++程序从OUTCAR提取坐标和力的数据。这是程序:

get_xf.cpp



13. 一些关于液态硅的计算结果

(0) Here are all the Matlab codes used to produce the following figures:


plot_temp.m

plot_rdf.m

plot_vac.m

find_rdf.m

find_vac_and_dos.m

(1) 首先，检查温度。我一开始用的SMASS=0，感觉效果不好。后来根据OUTCAR中给出的SMASS=0.19，我决定将SMASS改成0.19*200/40，代表热浴弛豫时间为步长的200倍。下面是温度演化图，感觉效果不错了。


temp.png





（2）再看看径向分布函数g(r)。VASP 有一个文件PCDAT包含了径向分布的函数的结果，但我还是喜欢自己算。用我另外一篇博文（http://blog.sciencenet.cn/blog-3102863-1084245.html ）的Matlab程序可以根据坐标和盒子信息算径向分布函数。直接上图：

gr.png

我将此图和文献【1】中的比较了一下，感觉差不多。

（3）速度自关联 (VAC)

VASP的输出文件里面没有每一步的速度。只能对坐标进行求导得到速度。不过，VASP输出的坐标是卷在盒子里的，首先要根据周期边界条件把坐标展开，然后对展开的坐标进行数值求导即可。下面是我计算的速度自关联函数：

VAC.png

(4) 扩散系数

跑动扩散系数（参见我以前的一篇博文http://blog.sciencenet.cn/blog-3102863-991891.html ）是速度自关联函数的时间积分再除以3（假设为各项同性系统）：

        D(t) = (1/3) * int_0^t VAC(t') dt'

这是我计算的跑动扩散系数随关联时间的变化图：

D.png

扩散系数收敛至 0.022 nm^2/ps = 2.2e-4 cm^2/s。作为比较，文献 [1] 的结果是2.26e-4 cm^2/s。还是比较接近的。

(5) 声子态密度 (PDOS)

对归一化的速度自关联进行离散傅里叶变换，可得到声子态密度，也叫做功率谱。用我之前一篇博文 (http://blog.sciencenet.cn/blog-3102863-1082676.html )中提供的程序，我计算出的声子态密度图如下：


PDOS.png

和文献[1]的也差不多（注意频率的单位换算）。



14. 参考文献

[1] I. Stich, R. Car, and M. Parrinello, Phys. Rev. B 44, 4262 (1991). 

https://doi.org/10.1103/PhysRevB.44.4262

