# Cuda-Q for 551

## Simple Installation (Only .py files)

Guides for setting up a Cuda-Q python environment can be found [here](https://nvidia.github.io/cuda-quantum/latest/using/quick_start.html)

If you just want to be able to run `.py` files, do the following:

1. Ensure you are connected to the GlobalProtect VPN (instructions [here](https://support.csuchico.edu/TDClient/1984/Portal/KB/?CategoryID=15690))
2. `ssh username@cscigpu.csuchico.edu`
    - Enter your password after OK-ing the signature.
3. `python -m pip install cuda-quantum` 
4. Try running program.py with `python program.py`

I would suggest the use of a python venv for package management. [Conda Docs](https://conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html), and [Example Venv Setups](https://janakiev.com/blog/jupyter-virtual-envs/) might be helpful.


## Advanced Setup - SSH Keys, Conda Environment, Jupyter Notebooks

### SSH Keys
1. On your local machine, generate an SSH Key with `ssh-keygen -t rsa -b 4096`
2. Keep track of where **id_rsa** and **id_rsa.pub** are saved.
3. Copy the contents of **id_rsa.pub** (found on your local machine).
4. Remote into cscigpu: `ssh username@cscigpu.csuchico.edu` (Again, make sure you're connected to the VPN).
5. Open `~/.ssh/authorized_keys` and paste the contents of your **id_rsa.pub** to a new line.
6. Now your machine can ssh into cscigpu more easily.

### Conda Environment (on cscigpu)
0. If you would rather install on your local machine, check resources at the bottom of the page.
1. Python environment set up: `conda env create --name cudaq --file=environment.yml`
    - If **cuda-quantum** and **contfrac** won't install through conda, run `conda run -n cudaq pip install cuda-quantum contfrac`
2. Python environment activated: `conda activate cudaq`
3. Verify ipykernel is installed: `pip install --user ipykernel`
4. Add python environment to Jupyter Server: `python -m ipykernel install --user --name=cudaq`

### Load Jupyter Server from Remote
1. After everything else has been set up and your environment is activated, do the following
2. On cscigpu, run `jupyter notebook --no-browser --port=8080`
3. On a seperate terminal in your local machine, set up an ssh tunnel:
    `ssh -L 8080:localhost:8080 username@cscigpu.csuchico.edu`
4. Now, open a browser on your local machine and go to [http://localhost:8080/](http://localhost:8080/)
5. This uses http, so a security warning may pop up, just click through it.
6. When asked for a token, copy the token from the URL that displayed when you started the jupyter server on cscigpu.

### Notes
- To close the Jupyter Server, hit ctrl+C and type 'y'.
- To delete a conda virtual environment (in case of mistakes / retrying), type `conda env remove -n cudaq`
    - `cudaq` may be named something differently if you named your virtual environment something else.

## Sources and Resources
1. [Cuda-Q: Shor's Algorithm](https://nvidia.github.io/cuda-quantum/latest/examples/python/tutorials/Shors.html)
2. [Cuda-Q: Quick Start](https://nvidia.github.io/cuda-quantum/latest/using/quick_start.html) 
3. [Cuda-Q: Academic Repo](https://github.com/NVIDIA/cuda-q-academic)
4. [Conda: Managing Environments](https://conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#activating-an-environment)
5. [Conda: Installation](https://docs.conda.io/projects/conda/en/4.6.1/user-guide/install/linux.html)
6. [SSH Using VSCode](https://help.rc.ufl.edu/doc/SSH_Using_VS_Code)
7. [StackOverflow: Accessing Jupyter Server on a remote machine](https://stackoverflow.com/questions/69244218/how-to-run-a-jupyter-notebook-through-a-remote-server-on-local-machine)