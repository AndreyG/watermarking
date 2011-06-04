cd tester && ant check-tests && cd .. &&
scala -classpath tester/classes TestChecker conf/gen-test.conf $1
