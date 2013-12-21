echo simple:
rm ./smsa.bin && ./smsasim simple.dat 2>&1 | ./verify simple-output.log

echo
echo linear:
rm ./smsa.bin && ./smsasim linear.dat 2>&1 | ./verify linear-output.log

echo
echo random:
rm ./smsa.bin && ./smsasim random.dat 2>&1 | ./verify random-output.log
