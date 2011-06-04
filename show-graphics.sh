cd tester && ant show-graphics && cd .. &&
scala -classpath tester/classes:tester/lib/jcommon-1.0.14.jar:tester/lib/jfreechart-1.0.11.jar GraphicViewer conf/gen-test.conf 
