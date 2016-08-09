/* Ahmed ELTantawy .. UBC */

#include "llvm/Analysis/SIMDDeadlockAnalysisNvidia.h"
using namespace llvm;


char SIMDDeadlockAnalysisNvidia::ID = 0;
INITIALIZE_PASS_BEGIN(SIMDDeadlockAnalysisNvidia, "simd-deadlock-analysis-nvidia", "SIMD Deadlock Analysis Info For Nvidia", true, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTree)
INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_END(SIMDDeadlockAnalysisNvidia, "simd-deadlock-analysis-nvidia", "SIMD Deadlock Analysis Info For Nvidia", true, true)



SIMDDeadlockAnalysisNvidia::SIMDDeadlockAnalysisNvidia() : ModulePass(ID)
{
	MainFunctions = new SmallVector<const Function*, MAX_MAIN_FUNCTIONS>;
	NonInlinedFunctions = new SmallVector<const Function*, MAX_MAIN_FUNCTIONS>;
	FunctionHelperInfos = new std::map <const Function *,FunctionHelperInfo*>;
	numLoops = new std::map<Function *,unsigned>;
	numBranches = new std::map<Function *,unsigned>;
	numCondBranches = new std::map<Function *,unsigned>;
	numLoopsDependantOnSharedMemory = new std::map<Function *,unsigned>;
	numLoopsDependantOnSharedMemoryWithinTheLoop = new std::map<Function *,unsigned>;
	numLoopsPotentiallyRedefined = new std::map<Function *,unsigned>;
	numLoopsPotentiallyRedefinedRelaxed = new std::map<Function *,unsigned>;
	numLoopsNeedsTransformation = new std::map<Function *,unsigned>;
	numberOfKernels=0;
}

void SIMDDeadlockAnalysisNvidia::getAnalysisUsage(AnalysisUsage &Info) const
{
	  Info.addRequired<PostDominatorTree>();
	  Info.addRequired<DominatorTreeWrapperPass>();
	  Info.addRequired<AliasAnalysis>();
	  Info.setPreservesAll();
}

bool SIMDDeadlockAnalysisNvidia::isMainFunction(Function *F)
{
	return (std::find(MainFunctions->begin(), MainFunctions->end(),F)!=MainFunctions->end());
}


bool SIMDDeadlockAnalysisNvidia::isNonInlinedFunction(Function *F)
{
	return (std::find(NonInlinedFunctions->begin(), NonInlinedFunctions->end(),F)!=NonInlinedFunctions->end());
}


void SIMDDeadlockAnalysisNvidia::extractMainFunctions(CallGraph * CG)
{

	for (auto it = CG->begin(), et = CG->end(); it != et; ++it){
		if((*it).second->getNumReferences()==1){
			MainFunctions->push_back((*it).first);
		}
	}
}


bool SIMDDeadlockAnalysisNvidia::runOnModule(Module & M)
{

	CallGraph cCG = CallGraph(M);
	CG = &cCG;
	extractMainFunctions(CG);
	AliasAnalysis * AA=&getAnalysis<AliasAnalysis>();
	for (Module::iterator it = M.begin(); it != M.end(); ++it) {
		Function *F = &*it;
		if (!F->isDeclaration()) {
			//assert(isMainFunction(F)); //Verifies that all kernels are exhaustively inlined.
			PostDominatorTree *PDT = &getAnalysis<PostDominatorTree>(*F);
			DominatorTreeWrapperPass *DTP = &getAnalysis<DominatorTreeWrapperPass>(*F);
			DominatorTree * DT  = &DTP->getDomTree();
			FunctionHelperInfo * FHI = new FunctionHelperInfo(F,AA,PDT,DTP,DT);
			(*FunctionHelperInfos)[F]=FHI;
			FHI->getConditionalBranchesInfo();
		}
	}
	dump(M);
	calculateStats(M);
	printStats(M);
	return false;
}

void SIMDDeadlockAnalysisNvidia::calculateStats(Module & M)
{
	for (Module::iterator it = M.begin(); it != M.end(); ++it) {
		Function *main = &*it;
		(*numBranches)[main]=0;
		(*numCondBranches)[main]=0;
		(*numLoops)[main]=0;
		(*numLoopsDependantOnSharedMemory)[main]=0;
		(*numLoopsDependantOnSharedMemoryWithinTheLoop)[main]=0;
		(*numLoopsPotentiallyRedefined)[main]=0;
		(*numLoopsNeedsTransformation)[main]=0;
		if (!main->isDeclaration()) {
			if(isMainFunction(main))
				numberOfKernels++;

			for(auto iter = main->begin(), eter = main->end(); iter!=eter; ++iter){
				BasicBlock *BB = iter;
				for(auto bbiter = BB->begin(), bbeter = BB->end(); bbiter!=bbeter; ++bbiter){
					Instruction *I = bbiter;
					if(dyn_cast<BranchInst>(I)){
						(*numBranches)[main]++;
						if(dyn_cast<BranchInst>(I)->isConditional()){
							(*numCondBranches)[main]++;
						}
					}
				}
			}

			FunctionHelperInfo *Finfo = FunctionHelperInfos->at(main);
			for (auto iter =Finfo->getRetreatingBranches()->begin(), eter = Finfo->getRetreatingBranches()->end(); iter != eter; ++iter){
				if((*iter)->createsLoop()){
					(*numLoops)[main]++;
					if((*iter)->isDependantOnSharedMemory()){
						(*numLoopsDependantOnSharedMemory)[main]++;
						if((*iter)->isDependantOnSharedMemoryWithinTheLoop()){
							(*numLoopsDependantOnSharedMemoryWithinTheLoop)[main]++;
							if((*iter)->isPotentiallyRedefined()){
								(*numLoopsPotentiallyRedefined)[main]++;
							}
							if((*iter)->isPotentiallyRedefinedRelaxed()){
								(*numLoopsPotentiallyRedefinedRelaxed)[main]++;
							}
							if((*iter)->doesNeedTransformation()){
								(*numLoopsNeedsTransformation)[main]++;
							}
						}
					}

				}
			}
		}
	}
}

void SIMDDeadlockAnalysisNvidia::printStats(Module & M)
{
	errs() << "SIMD Deadlock Analysis Stats:\n";
	errs() << "Number of Kernels: " << numberOfKernels << "\n";
	for (Module::iterator it = M.begin(); it != M.end(); ++it) {
		Function *main = &*it;
		if (!main->isDeclaration()) {
			errs()<< "Function Name: " << main->getName() << "\n";
			errs()<< "Number  Branches: " << (*numBranches)[main] << "\n";
			errs()<< "Number  Conditional Branches: " << (*numCondBranches)[main] << "\n";
			errs()<< "Number  Loops(Retreating Edges): " << (*numLoops)[main] << "\n";
			errs()<< "Number   dosh: " << (*numLoopsDependantOnSharedMemory)[main] << "\n";
			errs()<< "Number doshwl: " << (*numLoopsDependantOnSharedMemoryWithinTheLoop)[main] << "\n";
			errs()<< "Number potred: " << (*numLoopsPotentiallyRedefined)[main] << "\n";
			errs()<< "Number transf: " << (*numLoopsNeedsTransformation)[main] << "\n";
			errs()<< "Number relaxd: " << (*numLoopsPotentiallyRedefinedRelaxed)[main] << "\n";

		}
	}
}

void SIMDDeadlockAnalysisNvidia::dump(Module & M)
{
	//calculateStats(M);
	//printStats();
	errs() << "SIMD Deadlock Analysis Details:\n";
	for (Module::iterator it = M.begin(); it != M.end(); ++it) {
		Function *main = &*it;
		if (!main->isDeclaration()) {
			errs()<<"Function: " << main->getName() << "\n";
			FunctionHelperInfo *Finfo = FunctionHelperInfos->at(main);
			for (auto iter =Finfo->getRetreatingBranches()->begin(), eter = Finfo->getRetreatingBranches()->end(); iter != eter; ++iter){
				(*iter)->print();
			}
		}
	}

	for (Module::iterator it = M.begin(); it != M.end(); ++it) {
		Function *main = &*it;
		if (!main->isDeclaration()) {
			errs()<<"Function: " << main->getName() << "\n";
			FunctionHelperInfo *Finfo = FunctionHelperInfos->at(main);
			Finfo->print();
		}
	}
}
