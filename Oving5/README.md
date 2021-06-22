# Practical5


## Run
gcc benchmarking.c -o benchmarking
gcc benchmarkingNamed.c -o benchmarkingNamed

./benchmarking numberOfBytes
./benchmarkingNamed numberOfBytes

### Example

./benchmarking 12230
./benchmarkingNamed 12230

## Debug
gcc benchmarking.c -g

gdb ./a.out

run

## Processes
ps aux 
kill pid (pid first nr. from ps aux)

## Kill with USR1
 kill -s USR1 pid