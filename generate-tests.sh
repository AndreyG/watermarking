cd tester && ant && cd .. &&
scala -classpath tester/classes TestGenerator 0 build _ alpha-0.1 0 0 1 &&
chmod +x test-run.sh 
