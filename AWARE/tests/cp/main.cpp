// created based upon description in readme.rtf

#include <oclUtils.h>

#include "RopaHarness.h"
// #include <GL/glut.h>
#include <pthread.h>
#include <unistd.h>

#include <string>

RopaHarness *g_rh;
bool g_draw_done;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
int global_cycle = 0;

shrBOOL bQATest = shrFALSE;
shrBOOL bTM = shrFALSE;
shrBOOL bTMOpt = shrFALSE;
shrBOOL bAtomic = shrFALSE;
shrBOOL bEnableOutputDump = shrFALSE;
int num_cycles;
int local_worksize;

void Draw()
{
   g_rh->SetViewport(1000,1000);
   g_rh->ResetCamera();
   g_rh->Render();
   if(!bQATest) {
	   // glutSwapBuffers();
   }
}

void update(int value)
{
   printf(".");
   fflush(stdout);
   g_rh->ComputeFrame(++global_cycle);
   Draw();
   if(!bQATest && global_cycle < num_cycles - 1) {
	   // glutTimerFunc(1000,update,0);
   }

   if(!bQATest && global_cycle >= num_cycles - 1) {
	   printf("Finished %d cycles.\n", num_cycles);
   }
}

void Mouse( int button, int state, int x, int y)
{
   float sx = 10.0*(x-500.0)/200.0;
   float sy = 10.0*(y-500.0)/350.0;
   if( sx < -5.0 ) { sx= -5; }
   if( sx > 5.0 )  { sx= 5; }
   if( sy < -5.0 ) { sy= -5; }
   if( sy > 5.0 )  { sy= 5; }
   printf("x=%d, y=%d, sx=%f, sy=%f\n", x, y, sx, sy );
   g_rh->UserImpulse(sx,sy);
}

int main(int argc, const char **argv)
{
	// process command line arguments
   const char cl_kernel_path_default[] = "./ropa.cl"; 
   char *cl_kernel_path = (char *)cl_kernel_path_default; 
   const char input_data_path_default[] = "./ROPA_DATA/"; 
   char *input_data_path = (char *)input_data_path_default;
   const char input_data_default[] = "tshirt_1.bin";
   char *input_data = (char *)input_data_default;

	if (argc > 1)
	{
		bQATest = shrCheckCmdLineFlag(argc, argv, "qatest");
		bTM = shrCheckCmdLineFlag(argc, argv, "tm");
		bTMOpt = shrCheckCmdLineFlag(argc, argv, "tm_opt");
      bAtomic = shrCheckCmdLineFlag(argc, argv, "atomic");
		bEnableOutputDump = shrCheckCmdLineFlag(argc,argv, "odump");

		// Number of simulation cycles
		if( shrCheckCmdLineFlag(argc, argv, "cycles") ) {
			shrGetCmdLineArgumenti(argc, argv, "cycles", &num_cycles);
			if(num_cycles < 1)
				num_cycles = 1;
		} else {
			num_cycles = 1;
		}

		// Local worksize
		if( shrCheckCmdLineFlag(argc, argv, "worksize") ) {
         shrGetCmdLineArgumenti(argc, argv, "worksize", &local_worksize);
         assert(local_worksize > 0);
      } else {
         local_worksize = 256;
      }

      if (shrGetCmdLineArgumentstr(argc, argv, "clpath", &cl_kernel_path) == shrFALSE) {
         cl_kernel_path = (char *)cl_kernel_path_default; 
      } 
      if (shrGetCmdLineArgumentstr(argc, argv, "data", &input_data_path) == shrFALSE) {
         input_data_path = (char *)input_data_path_default; 
      }
      if (shrGetCmdLineArgumentstr(argc, argv, "input", &input_data) == shrFALSE) {
         input_data = (char *)input_data_default;
      }
	}

	if(bTM && bAtomic) {
	   printf("Error: cannot enable TM and Atomic at the same time.\n");
	   abort();
	}

   printf("Local worksize = %d\n", local_worksize);
	printf("Using TM = %s\n", (bTM?"yes":"no"));
	printf("Using TM (Optimized Kernel) = %s\n", (bTMOpt?"yes":"no"));
   printf("Using Atomic = %s\n", (bAtomic?"yes":"no"));
	printf("Using input data: %s\n", input_data);
	printf("Number of cycles = %u\n", num_cycles);
	printf("Dump output to file = %s\n", (bEnableOutputDump?"yes":"no"));
	printf("Loading CL source code from file = %s\n", cl_kernel_path);

   std::string strRopaCL(cl_kernel_path); 
   FILE *fp = fopen(strRopaCL.c_str(), "r");
   if( fp == NULL ) {
      printf("could not open %s\n", strRopaCL.c_str());
      exit(1);
   }
   fseek(fp,0,SEEK_END);
   unsigned len = ftell(fp);
   if( len == 0 ) {
      printf("error 2\n");
      exit(2);
   }
   fseek(fp,0,SEEK_SET);
   char *pgm = (char*) calloc(len+2,1);
   fread(pgm,1,len,fp);
  
   fclose(fp);
 
   if(!bQATest) {
	   // glutInit(&argc, (char**)argv);
	   // glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );
	   // glutInitWindowSize(1000,1000);
	   // glutCreateWindow("Testing Testing");
   }

   DistanceSolverOptions disSolverOpts; 
   disSolverOpts.useTM = bTM; 
   disSolverOpts.useTMOpt = bTMOpt; 
   disSolverOpts.useAtomic = bAtomic; 
    
   g_rh = new RopaHarness( NULL, pgm, disSolverOpts, local_worksize, (bool) bEnableOutputDump );
   g_rh->Render();
   std::string strInputDataPath(input_data_path); 
   strInputDataPath += "/" + std::string(input_data);
   g_rh->Load(strInputDataPath.c_str());
   g_rh->SetMode(true,1,true,true,false);
   Mouse(0,0,500,500);
   Draw();
   g_rh->ComputeFrame(0);

   if(!bQATest) {
	   // glutDisplayFunc(Draw);
	   // glutMouseFunc(Mouse);
	   // glutTimerFunc(16,update,0);

	   // glutMainLoop();
   } else {
	   for (int i = 1; i < num_cycles; i++)
	   {
		   printf("\n\nCycle %d\n", i+1);
		   update(0);
	   }
   }

   return 0;
}
