#include "llvm/Analysis/SIMDDeadlockAnalysisCommons.h"
#define NVIDIA_RECONVERGENCE
namespace llvm {
	class SIMDDeadlockAnalysisNvidia : public ModulePass
	{
		public:
			static char ID;
			explicit SIMDDeadlockAnalysisNvidia();
			bool runOnModule(Module &M) override;
			void extractMainFunctions(CallGraph *CG);
			void getAnalysisUsage(AnalysisUsage &Info) const override;
			void dump(Module & M);
			void calculateStats(Module & M);
			void printStats(Module & M);

			//Helper Functions
			bool isMainFunction(Function *F);
			bool isNonInlinedFunction(Function *F);
			SmallVector<const Function*, MAX_MAIN_FUNCTIONS> * MainFunctions;
			SmallVector<const Function*, MAX_MAIN_FUNCTIONS> * NonInlinedFunctions;
			map <const Function *,FunctionHelperInfo*> * FunctionHelperInfos;

			protected:


			CallGraph *CG;
			int numberOfKernels;
			map<Function *,unsigned> *numLoops;
			map<Function *,unsigned> *numBranches;
			map<Function *,unsigned> *numCondBranches;
			map<Function *,unsigned> *numLoopsDependantOnSharedMemory;
			map<Function *,unsigned> *numLoopsDependantOnSharedMemoryWithinTheLoop;
			map<Function *,unsigned> *numLoopsPotentiallyRedefined;
			map<Function *,unsigned> *numLoopsNeedsTransformation;
			map<Function *,unsigned> *numLoopsPotentiallyRedefinedRelaxed;
	};
}  // end of anonymous namespace


