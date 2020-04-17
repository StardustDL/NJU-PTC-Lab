for file in $(find A/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file
    echo
done

for file in $(find B/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file
    echo
done
