echo
echo "This SCRIPT is OBSOLETE.  Please use 'make'"
echo "   make"
echo "    (debug=1)? (os=win32)? (clean)?"
echo "    (rexx | yaxx | dectest | toolstest | all)"
exit

echo Making the reslib....
cd reslib
g++ -c *cpp

echo Making the toolslib....
cd ../toolslib
g++ -c *cpp
cd encoding
g++ -c *cpp
cd ..

echo Making the rexxlib....
cd ../rexxlib
g++ -c *cpp

echo Making the yasp3lib:
cd ../yasp3lib
g++ -c *cpp
cd syntax
echo syntax...
g++ -c *cpp
cd ../parser
echo parser...
g++ -c *cpp
cd ../model
echo model...
g++ -c *cpp
cd ../entity
echo entity...
g++ -c *cpp
cd ../element
echo element...
g++ -c *cpp
cd ..

# cd ../yasp3
# g++ -c *cpp

# cd ../rexx
# g++ -c *cpp

echo Making YAXX...
cd ../yaxx
g++ -c *cpp

echo All Done.


