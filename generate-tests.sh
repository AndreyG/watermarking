cd tester && ant && cd .. &&
scala -classpath tester/classes TestGenerator 1 rivers '.*1000' alpha-0.5 1.0 2.5 0.1 &&
chmod +x test-run.sh 
