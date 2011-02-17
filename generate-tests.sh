cd tester && ant && cd .. &&
scala -classpath tester/classes TestGenerator 10 river '.*' '.*' 0.0 2.5 0.1 &&
chmod +x test-run.sh 
