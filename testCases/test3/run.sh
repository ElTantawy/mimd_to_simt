nvcc -arch=sm_20 interacFixed.cu  -o out0 
cuobjdump -ptx out0 > out0.ptx
cuobjdump -sass out0 > out0.sass

nvcc -Xcicc -O0 -Xptxas -O0    -arch=sm_20 interacFixed.cu  -o out1
cuobjdump -ptx out1 > out1.ptx
cuobjdump -sass out1 > out1.sass


nvcc -Xcicc -O0  -arch=sm_20 interacFixed.cu   -o out2
cuobjdump -ptx out2 > out2.ptx
cuobjdump -sass out2 > out2.sass


nvcc -G -arch=sm_20 interacFixed.cu  -o out3
cuobjdump -ptx out3 > out3.ptx
cuobjdump -sass out3 > out3.sass
