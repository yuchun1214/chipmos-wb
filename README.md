# ChipMos WB station

## Setup virtual environment

```shell=
python3 -m venv venv
source venv/bin/activate # for Unix-like
python3 -m pip install -r requirements.txt
```

## Compile c++ program

```shell=
mkdir build
cd build/
cmake ..
make
```


## Execute

* Step 1 : Edit the files' path in `config.json`
* Step 2 : Ensure the `config.json` file, `main.py` and `main` are in the same directory
* Step 3 : Data preprocessing to generate the `config.csv` file.
```shell=
python3 main.py config.json
```
* Step 4: Execute
```shell=
./main config.csv
```
