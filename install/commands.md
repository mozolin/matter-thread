# Useful commands

- **1. DOCKER**  
- **2. PYTHON**  
  
### 1. DOCKER

- Restarting ALL (stopped and running) docker containers
~~~
sudo docker restart $(sudo docker ps -a -q)
~~~

- Error resolving "nmcheck,gnome.org": Temporary failure in name resolution
~~~
sudo docker network prune
~~~

### 2. PYTHON

- No module named 'construct'
~~~
python3 -m pip install construct  
~~~

- No module named 'stdnum'
~~~
python3 -m pip install python-stdnum
~~~


### 3. GIT
To ignore changes to file permissions (chmod), use the command in the current repository:
~~~
git config core.fileMode false
~~~
