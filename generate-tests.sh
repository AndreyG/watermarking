scalac -d tester -sourcepath tester tester/TestGenerator.scala &&
scala -classpath tester TestGenerator 30 build 1.5 2.1 0.1 &&
chmod +x test-run.sh 
