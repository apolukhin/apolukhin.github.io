echo "becnhmarking clang"
clang++-3.7 -std=c++14 -O2 ./main.cpp && nice -n -20 ./a.out > clang.txt && rm a.out

echo "becnhmarking gcc"
g++-5 -std=c++1y -O2 ./main.cpp && nice -n -20 ./a.out > gcc.txt && rm a.out
