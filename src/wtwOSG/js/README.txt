The client side interaction contains slightly altered code 
from osgjs by Cedric Pinson (http://osgjs.org/osgjs/).
Check the license file osgJS_License.txt

All OSGJS files are "compiled" into the osgJS.aggregated.js using 
Google closure compiler 

I used a 2010 version, as the others where not producing valid JS code:
http://closure-compiler.googlecode.com/files/compiler-20100330.zip

usage:
java.exe -jar compiler.jar --js_output_file=osgJS.js --js=osg.js --js=osgGA.js --js=Vec3.js --js=Matrix.js --js=Manipulator.js --js=FirstPersonManipulator.js --js=OrbitManipulator.js  