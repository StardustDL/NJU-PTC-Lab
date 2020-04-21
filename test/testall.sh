CODE=0

echo Unit
cd Unit
if ./run.sh ../../src; then
  true
else
  CODE=-1
fi
cd ..

echo Syntax
cd syntax
if ./run.sh ../../src/parser; then
  true
else
  CODE=-1
fi
cd ..

echo Semantics
cd semantics
if ./run.sh ../../src/parser; then
  true
else
  CODE=-1
fi
cd ..

echo IR
cd ir
if ./run.sh ../../src/parser; then
  true
else
  CODE=-1
fi
cd ..

exit $CODE