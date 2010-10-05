scalac -d tester -sourcepath tester tester/TestChecker.scala &&
scala -classpath tester TestChecker $1 
