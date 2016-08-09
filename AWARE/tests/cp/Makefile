
#include ../path_setup.mk 

OBJS = ConstraintsSets.o  Effect.o  main.o  RopaHarness.o

TARGET:=./EA_Ropa

$(TARGET): $(OBJS)
	g++ -v  -O0 -g3 -o $(TARGET) $(OBJS) -I$(CUDAHOME)/include/CL/ -I$(CUDAHOME)/include/C  -L$(NVIDIA_COMPUTE_SDK_LOCATION)/OpenCL/common/lib/ -L$(NVIDIA_COMPUTE_SDK_LOCATION)/shared/lib/ -loclUtil_x86_64 -lshrutil_x86_64 -lGL -lOpenCL 

#$(TARGET): $(OBJS)
	#g++ -v  -O0 -g3 -o $(TARGET) $(OBJS) -I$(CUDAHOME)/include/CL/ -I$(CUDAHOME)/include/C  -L$(NVIDIA_COMPUTE_SDK_LOCATION)/OpenCL/common/lib/ -L$(NVIDIA_COMPUTE_SDK_LOCATION)/shared/lib/ -loclUtil_x86_64 -lshrutil_x86_64 -lGL -lOpenCL -lGLU -lglut  

#EA_Ropa.pure: $(OBJS)
   #purify -best-effort g++ -v -g3 -o EA_Ropa.pure $(OBJS) -L/usr/lib64 -lGL -lOpenCL -lGLU -lglut 

.cpp.o:
	g++ -g3 -O0 -c -I$(CUDAHOME)/include/C -I$(NVIDIA_COMPUTE_SDK_LOCATION)/shared/inc/ -I$(NVIDIA_COMPUTE_SDK_LOCATION)/OpenCL/common/inc/ $< -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)
