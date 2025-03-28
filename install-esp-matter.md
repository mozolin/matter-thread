
During the installation of Esp-Matter, compilation may stop due to errors...
~~~
cd ~/esp-matter
...
./install.sh
...
~~~

In case of errors such as:  
> ...pw: command not found  
> ...pop_var_context: hed of shell_variables not a function context"  
> ...nonja: build stopped: subcommand failed"  
> ...etc...  

~~~
cd ~/esp-matter/connectedhomeip/connectedhomeip
git clean -Xdf
source ./scripts/bootstrap.sh
~~~
