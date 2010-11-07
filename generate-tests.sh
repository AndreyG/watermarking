cd tester && ant && cd .. &&
scala -classpath tester/classes TestGenerator 3 river '.*' alpha-0.5 1.0 2.5 0.1 &&
chmod +x test-run.sh 
