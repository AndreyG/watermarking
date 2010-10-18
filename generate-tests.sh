scalac -d tester -sourcepath tester tester/TestGenerator.scala &&
scala -classpath tester TestGenerator 100 build _ alpha-0.1 1.5 2.2 0.01 &&
chmod +x test-run.sh 
