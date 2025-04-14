# Simulating a Thread network with OpenThread
https://openthread.io/codelabs/openthread-simulation-posix/index.html#1  
  
Build a leader node:
~~~
git clone --recursive https://github.com/openthread/openthread.git
cd openthread
./script/bootstrap
./script/cmake-build simulation
./script/cmake-build posix -DOT_DAEMON=ON
./build/simulation/examples/apps/cli/ot-cli-ftd 1
~~~
OpenThread CLI - Leader:
~~~
dataset init new
dataset
~~~
> Network Key: e4344ca17d1dca2a33f064992f31f786  
> PAN ID: 0xc169  
~~~
dataset commit active
ifconfig up
thread start
state
~~~
> leader

Build a child node:
~~~
./build/simulation/examples/apps/cli/ot-cli-ftd 2
~~~
OpenThread CLI - Child:
~~~
dataset networkkey e4344ca17d1dca2a33f064992f31f786
dataset panid 0xc169
dataset commit active
ifconfig up
thread start
state
~~~
> child
~~~
state
~~~
> router

OpenThread CLI - Leader:
~~~
router table
~~~
> | ID | RLOC16 | Next Hop | Path Cost | LQI In | LQI Out | Age | Extended MAC     |  
> +----+--------+----------+-----------+--------+---------+-----+------------------+  
> | 20 | 0x5000 |       63 |         0 |      0 |       0 |   0 | 96da92ea13534f3b |  
> | 22 | 0x5800 |       63 |         0 |      3 |       3 |  23 | 5a4eb647eb6bc66c |  
  
