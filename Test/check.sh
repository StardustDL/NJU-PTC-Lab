for file in $(find *.cmm)
do
    name=$(echo $file | cut -d. -f1)
    echo "Compiling $name ..."
    ../Code/parser $file
    echo
done
