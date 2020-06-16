for file in $(find syntax/tests/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/ncc $file --syntax
    echo
done

for file in $(find semantics/tests/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/ncc $file --semantics
    echo
done

for file in $(find ir/tests/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/ncc $file --ir
    echo
done

for file in $(find asm/tests/*.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../src/ncc $file
    echo
done