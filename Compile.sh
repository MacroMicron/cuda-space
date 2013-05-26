rm Obj.o
rm Camera.o
rm MathCamera.o
rm MathLinear.o
rm Space.out

#nvcc with no optimisation - add flags: "-Xopencc -O0 -Xptxas -O0"
#with gdb - add flags: -g -O0

echo "     OBJ.C -> OBJ.O"
gcc -c Obj.c -o Obj.o
if [ $? ]; then echo "     OK"; else echo "     ERROR"; fi
echo ""

echo "     CAMERA.C -> CAMERA.O"
gcc -c Camera.c -o Camera.o
if [ $? ]; then echo "     OK"; else echo "     ERROR"; fi
echo ""

echo "     MATHCAMERA.C -> MATHCAMERA.O"
gcc -c MathCamera.c -o MathCamera.o
if [ $? ]; then echo "     OK"; else echo "     ERROR"; fi
echo ""

echo "     TASK.C -> TASK.O"
gcc -c Task.c -o Task.o
if [ $? ]; then echo "     OK"; else echo "     ERROR"; fi
echo ""

echo "     MATHLINEAR.CU -> MATHLINEAR.O"
nvcc -c MathLinear.cu -o MathLinear.o -arch=sm_12 
if [ $? ]; then echo "     OK"; else echo "     ERROR"; fi
echo ""

echo "     MAINUI.C + OBJ.O + CAMERA.O + MATHCAMERA.O + MATHLINEAR.O -> SPACE.OUT"
nvcc MainUI.c Task.o Camera.o MathCamera.o MathLinear.o Obj.o -o Space.out -I /usr/include/GL/ -L /usr/lib/x86_64-linux-gnu/ -lGL -lGLU -lglut -arch=sm_12
if [ $? ]; then echo "     OK"; else echo "     ERROR"; fi
echo ""


