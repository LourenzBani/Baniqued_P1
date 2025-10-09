# STDISC_P1_S19
P1 - Threaded Prime Number Search

## Build Instructions (Windows)

### Step 1: Configuration
Make sure you have a `config.txt` file with two numbers. Example:
```
4 1000
```

First number = number of threads, Second number = upper limit for prime search

### Step 2: Compile the Programs
Run these commands one by one:
```
g++ -std=c++11 -pthread -o variant1 variant1.cpp
g++ -std=c++11 -pthread -o variant2 variant2.cpp
g++ -std=c++11 -pthread -o variant3 variant3.cpp
g++ -std=c++11 -pthread -o variant4 variant4.cpp
```

### Step 3: Run the Programs
```
variant1.exe
variant2.exe
variant3.exe
variant4.exe
```
