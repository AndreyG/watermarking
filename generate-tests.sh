cd tester && ant && cd .. &&
scala -classpath tester/classes TestGenerator conf/gen-test.conf &&
chmod +x test-run.sh 
