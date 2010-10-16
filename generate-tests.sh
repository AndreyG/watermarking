scalac -d tester -sourcepath tester tester/TestGenerator.scala &&
scala -classpath tester TestGenerator 30 river 0.5 1.5 0.1 &&
chmod +x test-run.sh 
