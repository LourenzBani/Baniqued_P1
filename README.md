# P1 - Threaded Prime Number Search - Lourenz Jhay G. Baniqued S19

## Project Organization

```
STDISCM-Problem-Set-1-main/
├── variant1/
│   ├── prime_search.cpp
│   └── config.txt
├── variant2/
│   ├── prime_search.cpp
│   └── config.txt
├── variant3/
│   ├── prime_search.cpp
│   └── config.txt
├── variant4/
│   ├── prime_search.cpp
│   └── config.txt
└── README.md
```

## The Four Variants

### Variant 1: Immediate Print + Range Division
Divides the search range equally among threads. Primes are printed immediately when found.

### Variant 2: Immediate Print + Divisibility Testing
Distributes numbers round-robin across threads. Primes are printed immediately when found.

### Variant 3: Batch Print + Range Division
Divides the search range equally among threads. All results are collected and printed after threads complete.

### Variant 4: Batch Print + Divisibility Testing
Distributes numbers round-robin across threads. All results are collected and printed after threads complete.

## Configuration

Each variant has a `config.txt` file:
```
Threads = 4
Max Value = 1000
```

## How to Build and Run

Navigate to each variant directory and compile:

```bash
cd variant1
g++ -std=c++20 -O3 -pthread prime_search.cpp -o prime_search.exe
./prime_search.exe
```

```bash
cd variant2
g++ -std=c++20 -O3 -pthread prime_search.cpp -o prime_search.exe
./prime_search.exe
```

```bash
cd variant3
g++ -std=c++20 -O3 -pthread prime_search.cpp -o prime_search.exe
./prime_search.exe
```

```bash
cd variant4
g++ -std=c++20 -O3 -pthread prime_search.cpp -o prime_search.exe
./prime_search.exe
```
