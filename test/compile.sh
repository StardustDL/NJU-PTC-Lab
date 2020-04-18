for file in $(find A/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --syntax
    echo
done

for file in $(find B/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --semantics
    echo
done

for file in $(find C/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --ir
    echo
done