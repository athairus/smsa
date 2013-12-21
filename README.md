#SMSA
SMSA stands for "Serially Massive Storage Array".
This driver was written as part of a term project for Penn State's CMPSC311.

##Features
* 256 byte blocks
* 256 blocks per 'drum'
* 16 drums in the entire array
* 256*256*16 = 1MB of storage

##Building
You will need these packages (built on Ubuntu 13.04):

<tt>libgcrypt11-dev
build-essential</tt>

Then, all you have to do is invoke <tt>make</tt>.

##Running
Execute this command before running the simluator, smsasim:

<tt>./env.sh</tt>

You can run a verification test on all 3 available workload simulator files by running:

<tt>./test.sh</tt>

##Credits
All code except for the following:
<tt>smsa_driver.c</tt>
<tt>test.sh</tt>
<tt>*.me.log</tt>

was written by Patrick McDaniel, a professor at Penn State Univeristy.
