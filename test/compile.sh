for file in $(find syntax/tests/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --syntax
    echo
done

for file in $(find semantics/tests/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --semantics
    echo
done

for file in $(find ir/tests/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --ir
    echo
done