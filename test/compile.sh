for file in $(find syntax/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --syntax
    echo
done

for file in $(find semantics/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --semantics
    echo
done

for file in $(find ir/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/parser $file --ir
    echo
done