out_file_name=test-run.sh
if [[ -n "$1" ]]; then
    out_file_name=$1
fi

cd tester && ant generate-tests && cd .. &&
scala -classpath tester/classes TestGenerator conf/gen-test.conf $out_file_name &&
chmod +x $out_file_name 
