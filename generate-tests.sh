cd tester && ant && cd .. &&
scala -classpath tester/classes TestGenerator 3 river '.*1000' alpha-1.0 1.0 2.5 0.1 &&
chmod +x test-run.sh 
