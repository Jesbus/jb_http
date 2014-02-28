import ast, sys;

print "hello world<br>"

inputList = ast.literal_eval( sys.argv[1] )
print type( inputList )
print inputList
print "<br>"
inputList = ast.literal_eval( sys.argv[2] )
print type( inputList )
print inputList

print "test\nsdef";
print "<form method='POST'><input type='text' name='aklsdf'/><button type='submit'>asd</button></form>"

for (int i=0;i<10000;i++)
{
	print i
}
