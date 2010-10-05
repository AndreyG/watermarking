scalac -d tester -sourcepath tester tester/TestGenerator.scala &&
scala -classpath tester TestGenerator 1 rivers 0.0 0.1 0.1 &&
chmod +x test-run.sh 
