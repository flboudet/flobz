echo 
echo '==== BENCHING LOOPS ====' 
cat /proc/cpuinfo | grep "model name" 
cat /proc/cpuinfo | grep "MHz" 
date 
gcc -O0 test.c -o test_c
echo '==== C version ====' 
time ./test_c
echo '==== GoomSL version ===='
(cd .. && make && time ./goomsl bench/TEST.gsl)
