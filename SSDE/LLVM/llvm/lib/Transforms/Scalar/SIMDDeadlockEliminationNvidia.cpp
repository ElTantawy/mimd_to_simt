
/* @Ahmed ElTantawy, UBC */


#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/SIMDDeadlockAnalysisNvidia.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Pass.h"

using namespace llvm;

#define MAX_PREDECESSORS 10
namespace {
	struct SIMDDeadlockEliminationNvidia : public ModulePass {
		static char ID; // Pass identification, replacement for typeid
		SIMDDeadlockEliminationNvidia() : ModulePass(ID) {
			initializeSIMDDeadlockEliminationNvidiaPass(*PassRegistry::getPassRegistry());
		}

		bool runOnModule(Module& M) override;
		bool doInitialization(Module& M) override;

		BasicBlock* convertMultipleEntryToSingleEngtryStage1(BasicBlock *BB, FunctionHelperInfo* FnInfo);
		void convertMultipleEntryToSingleEngtryStage2(BasicBlock *Exit, BasicBlock* newHeader, FunctionHelperInfo* FnInfo);
		void getNewLoopEntries(std::unordered_set<BasicBlock*>* LoopBBs,std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>* EnttriesToPredsMap, FunctionHelperInfo* FnInfo);
		BasicBlock * mergeEntriesPredecessors(BasicBlock * curEntry,BasicBlock *newHeader, std::unordered_set<BasicBlock *>* Preds, FunctionHelperInfo* FnInfo);
		void mergeSwitchTargets(BasicBlock * Exit,BasicBlock *newCommonTarget, FunctionHelperInfo* FnInfo);
		void addMissingPHINodesEdgesSpecial1(BasicBlock *dst,BackwardBranchInfo *BckwrdBr,Value* cond,PHINode* phi,FunctionHelperInfo* fInfo);
		void addMissingPHINodesEdgesSpecial2(BasicBlock *dst, BasicBlock * new_src,FunctionHelperInfo* FnInfo);
		bool checkIfDestinationExists(SwitchInst * sw,BasicBlock *newdst);
		void addMissingPHINodesEdges(BasicBlock *dst, BasicBlock * new_src,FunctionHelperInfo* FnInfo);
		unsigned getDominanceScore(BasicBlock * curEntry,std::map<BasicBlock*,std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>*> * TargetsToEntriesMap, FunctionHelperInfo* FnInfo);

		void updatePHINodes(BasicBlock *dst, BasicBlock *src, BasicBlock * new_src);
		Value * getPHIEQ(Type * ty,BasicBlock * new_src,std::map<BasicBlock*,Value*> *phi_map,FunctionHelperInfo* FnInfo);


		void updateDominanceRelations(FunctionHelperInfo* FnInfo);
		void updateDominanceRelatios(FunctionHelperInfo* FnInfo, Value * Op,DominatorTree * DT);
		void updateDominanceRelatios(FunctionHelperInfo* FnInfo, Instruction * Iuser, Value * Op,DominatorTree * DT,std::map<BasicBlock*,Value*> *phi_map,std::unordered_set<BasicBlock*>* Visited);
		Instruction * recursiveDominanceUpdate(FunctionHelperInfo* FnInfo, BasicBlock *B, Instruction *Inst, Value * Op, DominatorTree * DT,std::map<BasicBlock*,Value*> *phi_map, std::unordered_set<BasicBlock*>*Visited, PHINode* curphin);
		Value * returnDominantEquivelent(FunctionHelperInfo* FnInfo, BasicBlock *B, Instruction *Inst, Value * Op, DominatorTree * DT,std::map<BasicBlock*,Value*> *phi_map,std::unordered_set<BasicBlock*> * Visited, PHINode* curphin);
		bool isPrecededBy(FunctionHelperInfo* FnInfo,BasicBlock *B1, BasicBlock *B2);

		bool cleanRedundantPHIs(BasicBlock *B);
		bool cleanRedundantPHIs(Function *F);
		bool isIdenticalPHIs(PHINode * I1, PHINode * I2);

		void getAnalysisUsage(AnalysisUsage& AU) const override {
		      AU.addRequired<SIMDDeadlockAnalysisNvidia>();
		      AU.addRequired<PostDominatorTree>();
		      AU.addRequired<DominatorTreeWrapperPass>();
		}
	};
}


char SIMDDeadlockEliminationNvidia::ID = 0;
INITIALIZE_PASS(SIMDDeadlockEliminationNvidia, "simd-deadlock-elimination-nvidia", "SIMD Deadlock Elimination Nvidia Pass", false, false)

bool SIMDDeadlockEliminationNvidia::doInitialization(Module& M) {

	for (Module::iterator it = M.begin(); it != M.end(); ++it) {
		Function *main = &*it;
		if (!main->isDeclaration()) {
			while(!cleanRedundantPHIs(main))
			;
		}
	}
	return false;
}
bool SIMDDeadlockEliminationNvidia::runOnModule(Module& M) {
	/*
	 * This pass implements Algorithm-3 (SIMT deadlocks elimination)
	 */

	SIMDDeadlockAnalysisNvidia * MA =&getAnalysis<SIMDDeadlockAnalysisNvidia>();
	for (Module::iterator it = M.begin(); it != M.end(); ++it) {
		Function *main = &*it;
		if (!main->isDeclaration()) {
			// Apply the algorithm for each function
			FunctionHelperInfo* fInfo = MA->FunctionHelperInfos->at(main);
			std::unordered_set<Instruction *> * visitedPDOMs = new std::unordered_set<Instruction *>;
			std::map<Instruction*,unsigned> * numEdges = new std::map<Instruction*,unsigned>;
			std::map<Instruction*,PHINode *> * phiInsts= new std::map<Instruction*,PHINode *>;
			std::map<Instruction*,SwitchInst *> * swInsts= new std::map<Instruction*,SwitchInst *>;
			std::map<Instruction*,BasicBlock *> * newBBs= new std::map<Instruction*,BasicBlock *>;
			std::map<Instruction*,std::map<BasicBlock *,BasicBlock *>> * SrcDstMap= new std::map<Instruction*,std::map<BasicBlock *,BasicBlock *>>;
			std::map<Instruction*,std::map<BasicBlock *,Value *>> * SrcValMap= new std::map<Instruction*,std::map<BasicBlock *,Value *>>;

			for(auto iter = fInfo->getRetreatingBranches()->begin(), itere = fInfo->getRetreatingBranches()->end(); iter!=itere; ++iter){
				/*
				 * Iterate through all retreating edges (i.e., all loop latches)
				 */

				BackwardBranchInfo * BckwrdBr = *iter;
				if(BckwrdBr->doesNeedTransformation()){
					/*
					 * Apply the transformation on the retreating edges labeled by the detection pass to be in a need for a transformation
					 * as they could potentially lead to a SIMT-induced deadlocks.
					 */
					Instruction * ReqPDOMInst = BckwrdBr->getFinlReqPDOM();
					BasicBlock * ReqPDOMBB = ReqPDOMInst->getParent();
					if(std::find(visitedPDOMs->begin(),visitedPDOMs->end(),ReqPDOMInst)==visitedPDOMs->end()){
						visitedPDOMs->insert(ReqPDOMInst);
						BasicBlock * newBB = nullptr;
						// Check if ReqPDOMInst is at the begining of a basicblock
						if(ReqPDOMInst==ReqPDOMBB->begin()){
							// ReqPDOMInst is at the begining of a basicblock
							std::string * labelName1 = new std::string(ReqPDOMBB->getName());
							labelName1->append("_new");
							BasicBlock :: iterator nonPHI = ReqPDOMBB->getFirstNonPHI();
							ReqPDOMBB->splitBasicBlock(nonPHI,labelName1->c_str());
							newBB=ReqPDOMBB;
						}else{
							// ReqPDOMInst is at the middle of a basicblock, hence splitting the basicblock is required
							std::string * labelName1 = new std::string(ReqPDOMBB->getName());
							labelName1->append("_new");
							BasicBlock :: iterator IterReqPDOM = ReqPDOMInst;
							newBB = ReqPDOMBB->splitBasicBlock(IterReqPDOM,labelName1->c_str());
							std::string * labelName2 = new std::string(ReqPDOMBB->getName());
							labelName2->append("_newSucc");
							newBB->splitBasicBlock(newBB->begin(),labelName2->c_str());
						}

						//Add PHI node and initialize it for the current predecessors
						BasicBlock :: iterator nonPHIit = newBB->getFirstNonPHI();
						IRBuilder<> brtmp1(newBB, nonPHIit);
						PHINode * phi = brtmp1.CreatePHI(Type::getInt32Ty(getGlobalContext()),MAX_PREDECESSORS);
						ConstantInt *zero = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),0);
    					for (auto it = pred_begin(newBB), et = pred_end(newBB); it != et; ++it)
    					{
    						BasicBlock * pred = *it;
    						phi->addIncoming(zero,pred);
    					}

						//Add switch
    					TerminatorInst * curTInst = newBB->getTerminator();
    					BasicBlock * defaultDst = curTInst->getSuccessor(0);
						BasicBlock :: iterator beforeLast = --(newBB->end());
						IRBuilder<> brtmp2(newBB, beforeLast);
						SwitchInst * sw = brtmp2.CreateSwitch(phi,defaultDst);


						//Delete the unconditional branch
						Instruction * uncondBr = --(newBB->end());
						uncondBr->eraseFromParent();

						(*phiInsts)[ReqPDOMInst] = phi;
						(*swInsts)[ReqPDOMInst]  = sw;
						(*numEdges)[ReqPDOMInst] = 1;
						(*newBBs)[ReqPDOMInst] = newBB;
					}

					//add incoming edges to the phi node
					assert(numEdges->find(ReqPDOMInst)!=numEdges->end());
					ConstantInt *val = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),numEdges->at(ReqPDOMInst));
					ConstantInt *zero = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),0);
					assert(phiInsts->find(ReqPDOMInst)!=phiInsts->end());
					assert(swInsts->find(ReqPDOMInst)!=swInsts->end());
					Value * cond = BckwrdBr->getBrInst()->getOperand(0);

					/*
					 * The code below deals with two special cases along with the general case.
					 *
					 * The first special case: happens if two backward branches share the same header (destination)
					 * the action taken in this case is to merge these backward branches before redirecting them to
					 *
					 * The second special case: happens if the not-taken edge of the backward branches points to the
					 * switch statement. Then the action taken is to add a select statement at the end of the basicblock
					 * that contains the not-taken edge to check whether in this execution of the branch we are looping or not
					 *
					 */

					//check if the new destination is an existing one in the switch statement
					if(checkIfDestinationExists(swInsts->at(ReqPDOMInst),BckwrdBr->getTakenBB())){
				    	/*
				    	 * 1. Create a merge block.
				    	 * 2. The merge block has phis that takes two incoming edges from the merged sources. The values of the phis
				    	 * are copied from the their old destination.
				    	 * 3. Remove from phi nodes in the old destination the incoming values from current branch.
				    	 * 4. Update the successors of the current branch and the old src (the one we are merging with) to point to the
				    	 * the new merged block.
				    	 * 5. Replace the incoming values from old src in the phi nodes in the switch block with phi nodes in the merged block
				    	 */
						std::map<BasicBlock *,BasicBlock *> curSrcDst = SrcDstMap->at(ReqPDOMInst);
						BasicBlock * commondst = nullptr;
						BasicBlock * oldsrc = nullptr;
						Value * oldValue = nullptr;
						for(auto it=curSrcDst.begin(), et=curSrcDst.end(); it!=et; it++){
							BasicBlock * src = it->first;
							BasicBlock * dst = it->second;
							if(dst==BckwrdBr->getTakenBB()){
								commondst = dst;
								oldsrc = src;
								oldValue =  (*SrcValMap)[ReqPDOMInst][src];
								(*SrcDstMap)[ReqPDOMInst].erase(src);
							}
						}
						BasicBlock* block = BasicBlock::Create(getGlobalContext(), "merge", main);

					    for(BasicBlock :: iterator mii = BckwrdBr->getTakenBB()->begin(); mii != BckwrdBr->getTakenBB()->end(); mii++) {
					        /// the first instructions are always phi functions.

					        if(isa<PHINode>(mii)) {
					        	PHINode * phI = dyn_cast<PHINode>(mii);
					        	BasicBlock :: iterator nonPHIit = block->getFirstNonPHI();
								IRBuilder<> brtmp1(block, nonPHIit);
								PHINode * newphi = brtmp1.CreatePHI(phI->getType(),MAX_PREDECESSORS);
								unsigned NUM = phI->getNumIncomingValues();
								unsigned toremove = (unsigned)-1;
								for (unsigned i=0; i<NUM;i++)
								{
									if(phI->getIncomingBlock(i)==newBBs->at(ReqPDOMInst)){
										newphi->addIncoming(phI->getIncomingValue(i),oldsrc);
										phI->setIncomingValue(i,newphi);
									}else if(phI->getIncomingBlock(i)==BckwrdBr->getSelfBB()){
										newphi->addIncoming(phI->getIncomingValue(i),BckwrdBr->getSelfBB());
										toremove=i;
									}
								}
								phI->removeIncomingValue(toremove,true);
					        }
					    }

					    for(unsigned i=0; i<oldsrc->getTerminator()->getNumSuccessors();i++){
					    	if(oldsrc->getTerminator()->getSuccessor(i)==newBBs->at(ReqPDOMInst)){
					    		oldsrc->getTerminator()->setSuccessor(i,block);
					    	}
					    }


						BranchInst * brInst = BckwrdBr->getBrInst();
						if(brInst->isConditional()){
							for(unsigned i=0; i<brInst->getNumSuccessors();i++){
								if(brInst->getSuccessor(i)==BckwrdBr->getTakenBB()){
									brInst->setSuccessor(i,block);
								}
							}
						}else{
							brInst->setSuccessor(0,block);
						}

			        	BasicBlock :: iterator blockend = block->end();
						IRBuilder<> brtmp1(block, blockend);
						brtmp1.CreateBr(newBBs->at(ReqPDOMInst));

						for(unsigned i=0; i<(*phiInsts)[ReqPDOMInst]->getNumIncomingValues();i++){
							if((*phiInsts)[ReqPDOMInst]->getIncomingBlock(i)==oldsrc){
								(*phiInsts)[ReqPDOMInst]->setIncomingBlock(i,block);
							}
						}

						(*SrcDstMap)[ReqPDOMInst][block]=commondst;
						(*SrcValMap)[ReqPDOMInst][block]=oldValue;

					}else if(BckwrdBr->getNotTakenBB()==newBBs->at(ReqPDOMInst)){
						/*
						 * a special case
						 * 1. add a select instruction at the end of BckwrdBr->getSelfBB()
						 * 2. set the PHINode val for the edge coming from BckwrdBr->getSelfBB() to the select instruction
						 * 3. replace the conditional branch of BckwrdBr with uncoditional branch with the target newBBs->at(ReqPDOMInst)
						 * 4. add swInst val for the edge coming from BckwrdBr->getNotTakenBB()
						 */
						(*numEdges)[ReqPDOMInst]=numEdges->at(ReqPDOMInst)+1;

						BasicBlock :: iterator beforeLast1 = --(BckwrdBr->getSelfBB()->end());
						IRBuilder<> brtmp3(BckwrdBr->getSelfBB(), beforeLast1);
						if(BckwrdBr->getNotTakenBB()==BckwrdBr->getTrueBB()){
							SelectInst *sI = dyn_cast<SelectInst>(brtmp3.CreateSelect(cond,zero,val));
							for(unsigned i=0; i<phiInsts->at(ReqPDOMInst)->getNumIncomingValues();i++){
								if(phiInsts->at(ReqPDOMInst)->getIncomingBlock(i)==BckwrdBr->getSelfBB()){
									phiInsts->at(ReqPDOMInst)->setIncomingValue(i,sI);
								}
							}
							(*SrcDstMap)[ReqPDOMInst][BckwrdBr->getSelfBB()]=BckwrdBr->getTakenBB();
							(*SrcValMap)[ReqPDOMInst][BckwrdBr->getSelfBB()]=sI;

						}else{
							SelectInst *sI = dyn_cast<SelectInst>(brtmp3.CreateSelect(cond,val,zero));
							for(unsigned i=0; i<phiInsts->at(ReqPDOMInst)->getNumIncomingValues();i++){
								if(phiInsts->at(ReqPDOMInst)->getIncomingBlock(i)==BckwrdBr->getSelfBB()){
									phiInsts->at(ReqPDOMInst)->setIncomingValue(i,sI);
								}
							}
							(*SrcDstMap)[ReqPDOMInst][BckwrdBr->getSelfBB()]=BckwrdBr->getTakenBB();
							(*SrcValMap)[ReqPDOMInst][BckwrdBr->getSelfBB()]=sI;
						}

						BasicBlock :: iterator beforeLast2 = --(BckwrdBr->getSelfBB()->end());
						IRBuilder<> brtmp4(BckwrdBr->getSelfBB(), beforeLast2);
						brtmp4.CreateBr(newBBs->at(ReqPDOMInst));
						Instruction * condBr = --(BckwrdBr->getSelfBB()->end());
						condBr->eraseFromParent();



						swInsts->at(ReqPDOMInst)->addCase(val,BckwrdBr->getTakenBB());
						updatePHINodes(BckwrdBr->getTakenBB(),BckwrdBr->getSelfBB(),newBBs->at(ReqPDOMInst));
						//Add undefined values for the new edges
						addMissingPHINodesEdgesSpecial1(newBBs->at(ReqPDOMInst),BckwrdBr,cond,phiInsts->at(ReqPDOMInst),fInfo);

					}else{
						/*
						 * general case, the one described in the paper (just update the target of the original branch)
						 */
						phiInsts->at(ReqPDOMInst)->addIncoming(val,BckwrdBr->getSelfBB());
						swInsts->at(ReqPDOMInst)->addCase(val,BckwrdBr->getTakenBB());
						(*numEdges)[ReqPDOMInst]=numEdges->at(ReqPDOMInst)+1;
						(*SrcDstMap)[ReqPDOMInst][BckwrdBr->getSelfBB()]=BckwrdBr->getTakenBB();
						(*SrcValMap)[ReqPDOMInst][BckwrdBr->getSelfBB()]=val;

						//Change the target in the original branch
						BranchInst * brInst = BckwrdBr->getBrInst();
						if(brInst->isConditional()){
							for(unsigned i=0; i<brInst->getNumSuccessors();i++){
								if(brInst->getSuccessor(i)==BckwrdBr->getTakenBB()){
									brInst->setSuccessor(i,newBBs->at(ReqPDOMInst));
								}
							}
						}else{
							brInst->setSuccessor(0,newBBs->at(ReqPDOMInst));
						}
						//update phi nodes in the destination with the replacement of the predecessors
						updatePHINodes(BckwrdBr->getTakenBB(),BckwrdBr->getSelfBB(),newBBs->at(ReqPDOMInst));
						//Add undefined values for the new edges
						addMissingPHINodesEdgesSpecial2(newBBs->at(ReqPDOMInst),BckwrdBr->getSelfBB(),fInfo);
					}

				}
			}
			/*
			 * Note that if the next code is not used (i.e., we are targeting the SIMT stack and not an Nvidia GPU) then
			 * the switch statement needs to be lowered to branches such that it diverges first to the default not-taken path.
			 *  This guarantees that the ReqPDOM to be the IPDOM of subsequent swith branches. However, this is not necessary if
			 *  the next code is used.
			 *
			 * The next part is necessary for Nvidia's GPUs because they have different reconvergence properties
			 * compared to the SIMT stack described in Wilson et al. Dynamic Warp Formation paper and modeled in
			 * GPGPUsim. While the details of Nvidia's reconvergence mechanism is not available, here is our best
			 * guess about the differences (see references in the paper).
			 * 1. For a branch to reconverge at its immediate postdominator, it needs to dominate this immediate
			 * postdominator point.
			 * 2. To reconverge after a loop, the loop must have a single entry.
			 * The next part of the code attempts to have these guarantees by:
			 * * converting the multiple backward edges from the switch statement into a single one to a block
			 *   thet dominates the old switch basicblock. This block will contains a swicth statement that
			 *   redirects the control flow graph.
			 * * making sure the new loop has a single entry (to the new "dominating" switch block), any other
			 *   entry to the loop will be merged into the dominating switch block.
			 *
			 */


			#ifdef NVIDIA_RECONVERGENCE

			for(auto iter = swInsts->begin(), eter=swInsts->end(); iter!=eter; ++iter){
				SwitchInst * curSw = iter->second;
				BasicBlock * curBB = curSw->getParent();
				/*
				 * First stage picks a dominating block (newHeader) to be the header
				 * of our new single entry loop. Other loop entries are merged into
				 * the newHeader.
				 * Second stage merges switch branches into one and adds a switch at a dominating
				 * basicblock instead (i.e.,the  newHeader).
				 *
				 */
				BasicBlock * newHeader = convertMultipleEntryToSingleEngtryStage1(curBB,fInfo);
				convertMultipleEntryToSingleEngtryStage2(curBB,newHeader,fInfo);
			}

			#endif
			/*
			 * The code at this point should be SIMT deadlock free. However, it could potentially have
			 * a problem that not all instructions dominating their uses. This should not be a problem
			 * because for the paths through which the instruction is not defined are fake paths that
			 * will not be taken. However, LLVM does not really know they are fake and it is a required
			 * condition for the next stages that all instructions to dominate their uses. The next call
			 * fixes this problem by simply adding phi nodes as necessary to make sure that all uses are
			 * dominated by their definitions
			 */
			updateDominanceRelations(fInfo);
		}
	}
	return false;
}


void SIMDDeadlockEliminationNvidia::mergeSwitchTargets(BasicBlock * BB,BasicBlock *newCommonHeader, FunctionHelperInfo* FnInfo)
{

	std::string * labelName1 = new std::string(newCommonHeader->getName());
	labelName1->append("_se");
	BasicBlock :: iterator nonPHI = newCommonHeader->getFirstNonPHI();
	newCommonHeader->splitBasicBlock(nonPHI,labelName1->c_str());

	//add a compare with zero instruction (for the phi the control the switch)
	TerminatorInst * TInst = BB->getTerminator();
	SwitchInst * exitSW = dyn_cast<SwitchInst>(TInst);
	Value * exitSWCond = exitSW->getCondition();
	ConstantInt *zero = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),0);


	//add a phi statement that takes the old switch phi if coming from the backward edge, and zeros for all other destination
	BasicBlock * defaultDst = newCommonHeader->getTerminator()->getSuccessor(0); //single successor
	BasicBlock :: iterator nonPHIit = newCommonHeader->getFirstNonPHI();
	IRBuilder<> brtmp1(newCommonHeader, nonPHIit);
	PHINode * phi = brtmp1.CreatePHI(Type::getInt32Ty(getGlobalContext()),MAX_PREDECESSORS);
	for (auto it = pred_begin(newCommonHeader), et = pred_end(newCommonHeader); it != et; ++it)
	{
		BasicBlock * pred = *it;
		if(pred!=BB)
			phi->addIncoming(zero,pred);
	}
	phi->addIncoming(exitSWCond,BB);
	SwitchInst * sw = brtmp1.CreateSwitch(phi,defaultDst);
	for(auto it=exitSW->case_begin(), et=exitSW->case_end();it!=et;it++){
		SwitchInst::CaseIt  caseIt = (*it);
		if(caseIt.getCaseSuccessor()!=newCommonHeader){
			sw->addCase(caseIt.getCaseValue(),caseIt.getCaseSuccessor());
			updatePHINodes(caseIt.getCaseSuccessor(),BB,newCommonHeader);
		}
	}
	Instruction * uncondBr = --(newCommonHeader->end());
	uncondBr->eraseFromParent();



	//replace the switch with a branch that branch to the exit if the comparison is true, otherwise branches to the newCommon header
	BasicBlock :: iterator beforeLast2 = --(BB->end());
	IRBuilder<> builder(BB, beforeLast2);
	CmpInst * compInst = dyn_cast<CmpInst>(builder.CreateICmpEQ(exitSWCond,zero));
	builder.CreateCondBr(compInst,exitSW->getDefaultDest(),newCommonHeader);
	exitSW->eraseFromParent();

	addMissingPHINodesEdges(newCommonHeader,BB,FnInfo);

}





bool SIMDDeadlockEliminationNvidia::checkIfDestinationExists(SwitchInst * sw,BasicBlock *newdst)
{
	for(auto it=sw->case_begin(), et=sw->case_end();it!=et;it++){
		SwitchInst::CaseIt  caseIt = (*it);
		BasicBlock * Target = caseIt.getCaseSuccessor();
		if(Target==newdst) return true;
	}
	return false;
}


void SIMDDeadlockEliminationNvidia::convertMultipleEntryToSingleEngtryStage2(BasicBlock *BB, BasicBlock * newHeader, FunctionHelperInfo* FnInfo)
{
	mergeSwitchTargets(BB,newHeader, FnInfo);
}



void SIMDDeadlockEliminationNvidia::getNewLoopEntries(std::unordered_set<BasicBlock*>* LoopBBs,std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>* EnttriesToPredsMap, FunctionHelperInfo* FnInfo)
{
	for(auto itIL =  LoopBBs->begin(), itILe =  LoopBBs->end(); itIL!=itILe; ++itIL){
		BasicBlock * curBB = *itIL;
		for (auto it = pred_begin(curBB), et = pred_end(curBB); it != et; ++it)
		{
			  BasicBlock* predBB = *it;
				if(!FnInfo->doesPathContainBB(predBB, LoopBBs)){
					if(EnttriesToPredsMap->find(curBB)==EnttriesToPredsMap->end()){
						(*EnttriesToPredsMap)[curBB] = new std::unordered_set<BasicBlock *>;
					}
					(*EnttriesToPredsMap)[curBB]->insert(predBB);
				}
		}
	}
}

unsigned SIMDDeadlockEliminationNvidia::getDominanceScore(BasicBlock * curEntry,std::map<BasicBlock*,std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>*> * TargetsToEntriesMap, FunctionHelperInfo* FnInfo)
{
	Function * F = FnInfo->getFunction();
	DominatorTreeWrapperPass *DTP = &getAnalysis<DominatorTreeWrapperPass>(*F);
	DominatorTree * DT = &DTP->getDomTree();
	unsigned score = 0;
	for(auto it=TargetsToEntriesMap->begin(), et=TargetsToEntriesMap->end();it!=et;it++){
		std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>* curTargetsToEntriesMap = it->second;
		for(auto itt=curTargetsToEntriesMap->begin(), ett=curTargetsToEntriesMap->end();itt!=ett;itt++){
			BasicBlock * Entry = itt->first;
			if(DT->dominates(curEntry->begin(),Entry->begin())){
				score++;
			}
		}
	}
	return score;
}

BasicBlock * SIMDDeadlockEliminationNvidia::convertMultipleEntryToSingleEngtryStage1(BasicBlock *BB, FunctionHelperInfo* FnInfo)
{
	/*
	 * The next piece of code selects the newHeader that will be used as the new loop
	 * header. Currently, it uses a heuristic approach to select between a basicblock
	 * between one of the switch destinations.
	 */
	TerminatorInst * TInst = BB->getTerminator();
	SwitchInst * exitSW = dyn_cast<SwitchInst>(TInst);
	std::map<BasicBlock*,std::unordered_set<BasicBlock*>*> * TargetsToPathsMap = new std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>;
	std::map<BasicBlock*,std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>*> * TargetsToEntriesMap = new std::map<BasicBlock*,std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>*>;
	for(auto it=exitSW->case_begin(), et=exitSW->case_end();it!=et;it++){
		SwitchInst::CaseIt  caseIt = (*it);
		BasicBlock * Target = caseIt.getCaseSuccessor();
		(*TargetsToPathsMap)[Target] = new std::unordered_set<BasicBlock*>;
		(*TargetsToEntriesMap)[Target]= new std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>;
		FnInfo->getPath(Target,BB,(TargetsToPathsMap)->at(Target));
		getNewLoopEntries((TargetsToPathsMap)->at(Target),(TargetsToEntriesMap)->at(Target),FnInfo);
	}
	std::map<BasicBlock* , unsigned> DominanceScoreMap;
	unsigned MaximumDominanceScore = 0;
	for(auto it=TargetsToEntriesMap->begin(), et=TargetsToEntriesMap->end();it!=et;it++){
		std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>* curTargetsToEntriesMap = it->second;
		for(auto itt=curTargetsToEntriesMap->begin(), ett=curTargetsToEntriesMap->end();itt!=ett;itt++){
			BasicBlock * curEntry = itt->first;
			unsigned score = getDominanceScore(curEntry,TargetsToEntriesMap,FnInfo);
			//more points if curEntry is a Target
			if(TargetsToEntriesMap->find(curEntry)!=TargetsToEntriesMap->end()){
				score++;
			}

			DominanceScoreMap[curEntry]=score;
			if(score>MaximumDominanceScore)
				MaximumDominanceScore = score;
		}
	}
	std::unordered_set<BasicBlock*> * CandidateHeaders = new std::unordered_set<BasicBlock*>;
	for(auto it=DominanceScoreMap.begin(), et=DominanceScoreMap.end(); it!=et; it++){
		if(it->second==MaximumDominanceScore){
			CandidateHeaders->insert(it->first);
		}
	}

	//Pick anyone
	BasicBlock * newHeader = (*CandidateHeaders->begin());
	std::unordered_set<BasicBlock *> * newLoopBBs = new std::unordered_set<BasicBlock *>;
	std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>* newLoopEntriesToPredMap = new std::map<BasicBlock*,std::unordered_set<BasicBlock*>*>;
	FnInfo->getPath(newHeader,BB,newLoopBBs);
	getNewLoopEntries(newLoopBBs,newLoopEntriesToPredMap,FnInfo);


	if(newLoopEntriesToPredMap->size()==1){
		if(newLoopEntriesToPredMap->begin()->first==newHeader){
			return newHeader;
		}
	}

	/*
	 * The next piece of code uses the newHeader selected and forces any other entries to the
	 * looo dominated by newHeader to be merged into the newHeader (i.e., a single entry loop)
	 */

	//split the inteded header
	std::string * labelName1 = new std::string(newHeader->getName());
	labelName1->append("commonHeader");
	BasicBlock :: iterator nonPHI = newHeader->getFirstNonPHI();
	newHeader->splitBasicBlock(nonPHI,labelName1->c_str());

	//merge Preds of different entries into one
	std::map<BasicBlock*,BasicBlock*> * EntriesToMergedPredMap = new  std::map<BasicBlock*,BasicBlock*>;
	for(auto it=newLoopEntriesToPredMap->begin(), et=newLoopEntriesToPredMap->end();it!=et;it++){
		BasicBlock * curEntry = it->first;
		std::unordered_set<BasicBlock*>* Preds = it->second;
		if(curEntry!=newHeader){
			BasicBlock * mergedPredBB = mergeEntriesPredecessors(curEntry,newHeader,Preds,FnInfo);
			(*EntriesToMergedPredMap)[curEntry]=mergedPredBB;
		}
	}

	//existing phi nodes will need to add a new edge for each new entry for the loop
	for(auto it = newLoopEntriesToPredMap->begin(), et = newLoopEntriesToPredMap->end(); it!=et; ++it){
		BasicBlock *curEntry = it->first;
		if(curEntry != newHeader){
			BasicBlock * pred = (*EntriesToMergedPredMap)[curEntry];
			addMissingPHINodesEdges(newHeader,pred,FnInfo);
		}
	}

	//add phi instructin with default as zero
	unsigned trackEntries = 0;
	std::map<BasicBlock *, unsigned> *entryToUnsigned = new std::map<BasicBlock *, unsigned>;
	BasicBlock :: iterator nonPHIit = newHeader->getFirstNonPHI();
	IRBuilder<> brtmp1(newHeader, nonPHIit);
	PHINode * phi = brtmp1.CreatePHI(Type::getInt32Ty(getGlobalContext()),MAX_PREDECESSORS);
	ConstantInt *zero = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),0);
	for (auto it = pred_begin(newHeader), et = pred_end(newHeader); it != et; ++it)
	{
		BasicBlock * pred = *it;
		phi->addIncoming(zero,pred);
	}
	(*entryToUnsigned)[newHeader] = 0;
	trackEntries++;

	//add phi instruction
	for(auto it = EntriesToMergedPredMap->begin(), et = EntriesToMergedPredMap->end(); it!=et; ++it){
		BasicBlock *curEntry = it->first;
		BasicBlock *curPred = it->second;
		(*entryToUnsigned)[curEntry] = trackEntries;
		ConstantInt *cnst = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),(*entryToUnsigned)[curEntry]);
		phi->addIncoming(cnst,curPred);
		trackEntries++;
	}

	//Add switch
	TerminatorInst * curTInst = newHeader->getTerminator();
	BasicBlock * defaultDst = curTInst->getSuccessor(0);
	BasicBlock :: iterator beforeLast = --(newHeader->end());
	IRBuilder<> brtmp2(newHeader, beforeLast);
	SwitchInst * sw = brtmp2.CreateSwitch(phi,defaultDst);
	for(auto it = EntriesToMergedPredMap->begin(), et = EntriesToMergedPredMap->end(); it!=et; ++it){
		BasicBlock *curEntry = it->first;
		ConstantInt *cnst = ConstantInt::get(Type::getInt32Ty(getGlobalContext()),(*entryToUnsigned)[curEntry]);
		sw->addCase(cnst,curEntry);
	}

	//Delete the unconditional branch
	Instruction * uncondBr = --(newHeader->end());
	uncondBr->eraseFromParent();


	//update the targets of the old loop entries predecessors
	for(auto it = EntriesToMergedPredMap->begin(), et = EntriesToMergedPredMap->end(); it!=et; ++it){
		BasicBlock *curEntry = it->first;
		BasicBlock *curPred = it->second;
		std::unordered_set<BasicBlock*>* Preds = newLoopEntriesToPredMap->at(curEntry);
		if(curEntry != newHeader){
			if(Preds->size()>1){
				IRBuilder<> builder(curPred);
				builder.CreateBr(newHeader);
			}else{
				//update the target for the  old entries predecessors
				TerminatorInst *TInst = dyn_cast<TerminatorInst>(curPred->getTerminator());
				for(unsigned i=0; i<TInst->getNumSuccessors();i++){
					if(TInst->getSuccessor(i)==curEntry){
						TInst->setSuccessor(i,newHeader);
						updatePHINodes(curEntry,curPred,newHeader);
					}
				}
			}
		}
	}

	return newHeader;

}

BasicBlock* SIMDDeadlockEliminationNvidia:: mergeEntriesPredecessors(BasicBlock * curEntry,BasicBlock *newHeader, std::unordered_set<BasicBlock *>* Preds, FunctionHelperInfo* FnInfo)
{
	if(Preds->size()>1){
		BasicBlock* block = BasicBlock::Create(getGlobalContext(), "merge", FnInfo->getFunction());
		IRBuilder<> builder(block);
		std::map<PHINode* ,PHINode* > * phnodesmap = new std::map<PHINode* ,PHINode* >;
    	//1- Create a similar PhI node in block
	    for(BasicBlock :: iterator mii = curEntry->begin(); mii != curEntry->end(); mii++) {
	        /// the first instructions are always phi functions.
	        if(isa<PHINode>(mii)) {
	        	PHINode * phI = dyn_cast<PHINode>(mii);
	        	BasicBlock :: iterator nonPHIit;
	        	if(block->getFirstNonPHI())
	        		nonPHIit = block->getFirstNonPHI();
	        	else
	        		nonPHIit = block->begin();				
				IRBuilder<> brtmp1(block, nonPHIit);
				PHINode * newphi = brtmp1.CreatePHI(phI->getType(),MAX_PREDECESSORS);
				(*phnodesmap)[phI] = newphi;
				for (auto it = Preds->begin(), et = Preds->end(); it != et; ++it)
				{
					BasicBlock * pred = *it;
					newphi->addIncoming(phI->getIncomingValueForBlock(pred),pred);
				}
	        }
	    }

		//2- for all edges in curEntry coming from Preds, replace one of them by header and remove the others
	    for(BasicBlock :: iterator mii = curEntry->begin(); mii != curEntry->end(); mii++) {
	        /// the first instructions are always phi functions.
	        if(isa<PHINode>(mii)) {
	        	PHINode * phI = dyn_cast<PHINode>(mii);
				for (auto it = Preds->begin(), et = Preds->end(); it != et; ++it)
				{
					BasicBlock * pred = *it;
					if(pred == (*Preds->begin())){
						for(unsigned i=0; i<phI->getNumIncomingValues();i++){
							if(phI->getIncomingBlock(i)==pred){
								phI->setIncomingBlock(i,newHeader);
								phI->setIncomingValue(i,phnodesmap->at(phI));
							}
						}
					}else{
						for(unsigned i=0; i<phI->getNumIncomingValues();i++){
							if(phI->getIncomingBlock(i)==pred){
								phI->removeIncomingValue(i,true);
							}
						}
					}
				}
	        }
	    }


	    //3- replace the target of Preds terminator from curEntry to block
		for (auto it = Preds->begin(), et = Preds->end(); it != et; ++it)
		{
			BasicBlock * pred = *it;
			TerminatorInst * TI = pred->getTerminator();
			for(unsigned i=0; i<TI->getNumSuccessors();i++){
				if(TI->getSuccessor(i)==curEntry){
					TI->setSuccessor(i,block);
				}
			}
		}
		return block;
	}else{
		assert(Preds->size()==1);
		BasicBlock * pred = (*Preds->begin());
		updatePHINodes(curEntry,pred,newHeader);
		return pred;
	}

}

Value * SIMDDeadlockEliminationNvidia::getPHIEQ(Type * ty,BasicBlock * new_src,std::map<BasicBlock*,Value*> *phi_map,FunctionHelperInfo* FnInfo)
{
	bool undef = true;
	for(auto it=phi_map->begin(), et=phi_map->end(); it!=et; ++it){
		if(isPrecededBy(FnInfo,new_src,it->first)){
			undef = false;
		}
	}
	if(phi_map->find(new_src)!=phi_map->end()){
		return phi_map->at(new_src);
	}else if(undef){
		return UndefValue::get(ty);
	}else{
		IRBuilder<> phiop(new_src, new_src->begin());
		PHINode * phn = phiop.CreatePHI(ty,MAX_PREDECESSORS);
		(*phi_map)[new_src] = phn;
		for (auto it = pred_begin(new_src), et = pred_end(new_src); it != et; ++it)
		{
			BasicBlock* edge = *it;
			Value * val = nullptr;
			val = getPHIEQ(ty,edge,phi_map,FnInfo);
			phn->addIncoming(val,edge);
		}
		return phn;
	}
}




void SIMDDeadlockEliminationNvidia::updatePHINodes(BasicBlock *dst, BasicBlock *src, BasicBlock * new_src)
{
    for(BasicBlock :: iterator mii = dst->begin(); mii != dst->end(); mii++) {
        /// the first instructions are always phi functions.
        if(isa<PHINode>(mii)) {
        	PHINode * phI = dyn_cast<PHINode>(mii);
        	for (unsigned u = 0; u != phI->getNumIncomingValues(); ++u) {
        		if (phI->getIncomingBlock(u) == src) {
        			phI->setIncomingBlock(u,new_src);
        		}
        	}
        }
    }

}

void SIMDDeadlockEliminationNvidia::addMissingPHINodesEdges(BasicBlock *dst, BasicBlock * new_src,FunctionHelperInfo* FnInfo)
{

    for(auto mii = dst->begin(), mie=dst->end(); mii!=mie; ++mii) {
        /// the first instructions are always phi functions.
        if(isa<PHINode>(mii)) {
        	PHINode * phI = dyn_cast<PHINode>(mii);
        	bool exit = false;
        	for(unsigned i=0; i<phI->getNumIncomingValues();i++){
        		if(phI->getIncomingBlock(i)==new_src){
        			exit = true;
        		}
        	}

        	if(!exit){

        		std::map<BasicBlock*,Value*> *phi_map = new std::map<BasicBlock*,Value*>;
        		(*phi_map)[phI->getParent()] = phI;
        		for(unsigned i=0; i<phI->getNumIncomingValues();i++){
        			if(dyn_cast<PHINode>(phI->getIncomingValue(i))){
        				(*phi_map)[phI->getIncomingBlock(i)] = phI->getIncomingValue(i);
        			}
        		}


        		Type * ty = phI->getType();
        		Value * new_val = getPHIEQ(ty,new_src,phi_map,FnInfo);
            	phI->addIncoming(new_val,new_src);
        	}
        }
    }

}


void SIMDDeadlockEliminationNvidia::addMissingPHINodesEdgesSpecial1(BasicBlock *dst,BackwardBranchInfo *BckwrdBr,Value * cond,PHINode* Swphi, FunctionHelperInfo* fInfo)
{
	BasicBlock * new_src = BckwrdBr->getSelfBB();
	BasicBlock :: iterator beforeLast1 = --(new_src->end());
	IRBuilder<> brtmp3(new_src, beforeLast1);

	for(auto mii = dst->begin(), mie=dst->end(); mii!=mie; ++mii) {
        /// the first instructions are always phi functions.
        if(isa<PHINode>(mii)) {
        	PHINode * phI = dyn_cast<PHINode>(mii);
        	if(phI == Swphi) continue;
        	int index = -1;
			for(unsigned i=0; i<phI->getNumIncomingValues();i++){
				if(phI->getIncomingBlock(i)==BckwrdBr->getSelfBB()){
					index = i;
					if(BckwrdBr->getNotTakenBB()==BckwrdBr->getTrueBB()){
						SelectInst *sI = dyn_cast<SelectInst>(brtmp3.CreateSelect(cond,phI->getIncomingValue(index),UndefValue::get(phI->getType())));
						phI->setIncomingValue(index,sI);
					}else{
						SelectInst *sI = dyn_cast<SelectInst>(brtmp3.CreateSelect(cond,UndefValue::get(phI->getType()),phI->getIncomingValue(index)));
						phI->setIncomingValue(index,sI);
					}
				}
			}
        }
    }

}

void SIMDDeadlockEliminationNvidia::addMissingPHINodesEdgesSpecial2(BasicBlock *dst, BasicBlock * new_src,FunctionHelperInfo* FnInfo)
{

    for(auto mii = dst->begin(), mie=dst->end(); mii!=mie; ++mii) {
        /// the first instructions are always phi functions.
        if(isa<PHINode>(mii)) {
        	PHINode * phI = dyn_cast<PHINode>(mii);
        	bool exit = false;
        	for(unsigned i=0; i<phI->getNumIncomingValues();i++){
        		if(phI->getIncomingBlock(i)==new_src){
        			exit = true;
        		}
        	}

        	if(!exit){
            	phI->addIncoming(UndefValue::get(phI->getType()),new_src);
        	}
        }
    }

}



//Checks if B1 preceeded bu B2
bool SIMDDeadlockEliminationNvidia::isPrecededBy(FunctionHelperInfo* FnInfo, BasicBlock *B1, BasicBlock *B2)
{
	std::unordered_set<BasicBlock *> * V = new std::unordered_set<BasicBlock *>;
	FnInfo->getBackwardPath(B1,B2,V);
	if(!V->empty())
		return true;
	else
		return false;
}



Value * SIMDDeadlockEliminationNvidia::returnDominantEquivelent(FunctionHelperInfo* FnInfo,BasicBlock *B, Instruction *Inst, Value * Op, DominatorTree * DT,std::map<BasicBlock*,Value*> *phi_map, std::unordered_set<BasicBlock*> * Visited, PHINode* curphin)
{

	if(phi_map->find(B)!=phi_map->end()){
		return (Instruction *)phi_map->at(B);
	}else if(!isPrecededBy(FnInfo,B,Inst->getParent())){
		return UndefValue::get(Op->getType());
	}else{
		return recursiveDominanceUpdate(FnInfo,B,Inst,Op,DT,phi_map,Visited,curphin);
	}
}


Instruction * SIMDDeadlockEliminationNvidia::recursiveDominanceUpdate(FunctionHelperInfo* FnInfo,BasicBlock *B, Instruction *Inst, Value * Op, DominatorTree * DT,std::map<BasicBlock*,Value*> *phi_map, std::unordered_set<BasicBlock*>*Visited, PHINode* curphin)
{
	IRBuilder<> phiop(B, B->begin());
	PHINode * phn = phiop.CreatePHI(Op->getType(),MAX_PREDECESSORS);
	(*phi_map)[B] = phn;
	for (auto it = pred_begin(B), et = pred_end(B); it != et; ++it)
	{
		BasicBlock* edge = *it;
		Value * val = nullptr;
		val = returnDominantEquivelent(FnInfo,edge,Inst,Op,DT,phi_map,Visited,phn);
		phn->addIncoming(val,edge);
	}
	return phn;
}



void SIMDDeadlockEliminationNvidia::updateDominanceRelatios(FunctionHelperInfo* FnInfo,Instruction * Iuser, Value * Op, DominatorTree * DT,std::map<BasicBlock*,Value*> *phi_map,std::unordered_set<BasicBlock*>* Visited)
{
    if(!isa<PHINode>(Iuser)){
    	if(DT->dominates(dyn_cast<Instruction>(Op),Iuser))
    	{
    		/*errs() << "already dominates\n";*/ return;
    	}
    }else{
		PHINode * phnn = dyn_cast<PHINode>(Iuser);
		bool flag = false;
		for(unsigned i=0; i<phnn->getNumIncomingValues();i++){
			if(phnn->getIncomingValue(i)!=Op) continue;
			BasicBlock * curBB = phnn->getIncomingBlock(i);
			Instruction * curInst = dyn_cast<Instruction> (Op);
			if(!(DT->dominates(curInst,curBB)) && !(curInst->getParent()==curBB)){
				flag = true;
			}
		}
		if(!flag)
			return;
    }


    /*
     * All users that passed this point are not dominate their operands
     * (i.e., Op does not dominate Iuser)
     */

	if(!isa<PHINode>(Iuser)){
    	if(!DT->dominates(dyn_cast<Instruction>(Op),Iuser)){
    		(*phi_map)[dyn_cast<Instruction>(Op)->getParent()] = Op;
    		// Recursively update dominance relations
    		Instruction * phn =recursiveDominanceUpdate(FnInfo,Iuser->getParent(),dyn_cast<Instruction>(Op),Op,DT,phi_map,Visited,nullptr);
			for(unsigned i=0; i<Iuser->getNumOperands();i++){
				if(Iuser->getOperand(i)==Op){
	    			Iuser->setOperand(i,phn);
				}
			}
    		assert(DT->dominates(phn,Iuser));
    	}
	}else{
		PHINode * phnn = dyn_cast<PHINode>(Iuser);
		for(unsigned i=0; i<phnn->getNumIncomingValues();i++){
			if(phnn->getIncomingValue(i)!=Op) continue;
			(*phi_map)[Iuser->getParent()] = Iuser;
			(*phi_map)[dyn_cast<Instruction>(Op)->getParent()] = Op;

			BasicBlock * curBB = phnn->getIncomingBlock(i);
			Instruction * curInst = dyn_cast<Instruction> (Op);
			if(!(DT->dominates(curInst,curBB)) && !(curInst->getParent()==curBB)){
				Instruction * phn =recursiveDominanceUpdate(FnInfo,curBB,curInst,Op,DT,phi_map,Visited,nullptr);
    	    	Iuser->setOperand(i,phn);
	    		assert(((DT->dominates(phn,curBB)) || (phn->getParent()==curBB)));
			}
		}
	}
}

void SIMDDeadlockEliminationNvidia::updateDominanceRelatios(FunctionHelperInfo* FnInfo,Value * Op, DominatorTree * DT)
{
	std::map<BasicBlock*,Value*> *phi_map = new std::map<BasicBlock*,Value*>;
	std::unordered_set<BasicBlock*> *Visited = new std::unordered_set<BasicBlock*>;
	std::unordered_set<Value *> *Users = new std::unordered_set<Value*>;


	for (auto it = Op->user_begin(), et = Op->user_end(); it != et; ++it)
	{
		if(std::find(Users->begin(),Users->end(),(*it))==Users->end()){
			Users->insert(*it);
		}
	}


	for (auto it = Users->begin(), et = Users->end(); it != et; ++it)
	{
		if(isa<Instruction>(*it)){
			Instruction * Iuser = dyn_cast<Instruction>(*it);
			updateDominanceRelatios(FnInfo,Iuser,Op,DT,phi_map,Visited);
		}
	}
}


void SIMDDeadlockEliminationNvidia::updateDominanceRelations(FunctionHelperInfo* FnInfo)
{

	Function * F = FnInfo->getFunction();
	DominatorTreeWrapperPass *DTP = &getAnalysis<DominatorTreeWrapperPass>(*F);
	DominatorTree * DT = &DTP->getDomTree();

	std::map<Value *,std::map<BasicBlock *,PHINode *> * > * Vmap = new std::map<Value *,std::map<BasicBlock *,PHINode *> * >;
	Vmap->clear();
	std::unordered_set<Value *> * allValues = new std::unordered_set<Value *> ;
	for (auto I = F->begin(), E = F->end(); I != E; ++I)
     {
        for (auto II = I->begin(), IE = I->end(); II != IE; ++II)
        {
			Value*  Op = II;
			allValues->insert(Op);
        }
     }

    for (auto I = allValues->begin(), IE = allValues->end(); I != IE; ++I)
    {
	    Value * Op = (*I);
	    updateDominanceRelatios(FnInfo,Op,DT);
    }

    std::unordered_set<Instruction *> * toRemoveInsts = new std::unordered_set<Instruction *>;
	for (auto I = F->begin(), E = F->end(); I != E; ++I)
     {
        for (auto II = I->begin(), IE = I->end(); II != IE; ++II)
        {
        	if(dyn_cast<PHINode>(II)){
        		PHINode * phi = dyn_cast<PHINode>(II);
        		if(phi->getNumOperands()==0){
        			phi->replaceAllUsesWith(UndefValue::get(phi->getType()));
        			toRemoveInsts->insert(phi);
        		}
        	}
        }
     }

    for (auto I = toRemoveInsts->begin(), IE = toRemoveInsts->end(); I != IE; ++I)
    {
	    Instruction * Inst = (*I);
	    if(Inst->hasNUsesOrMore(1)){
			for (auto it =Inst->user_begin(), et = Inst->user_end(); it != et; ++it)
			{
				Instruction * Iuser = dyn_cast<Instruction>(*it);
				Iuser->dump();
			}
			abort();
	    }

	    if(Inst->getParent()) //otherwise, it is already removed
	    	Inst->eraseFromParent();
    }


	// Clean Redendant PHIs
	while(!cleanRedundantPHIs(F))
	;

}


bool SIMDDeadlockEliminationNvidia::isIdenticalPHIs(PHINode * I1, PHINode * I2)
{
	if(I1->getType()!=I2->getType())
		return false;
	if(I1->getNumOperands()!=I2->getNumOperands())
		return false;

	std::map<BasicBlock *, Value *> phi1;
	for(unsigned i=0; i<I1->getNumOperands(); i++){
		phi1[I1->getIncomingBlock(i)]=I1->getIncomingValue(i);
	}


	for(unsigned i=0; i<I2->getNumOperands(); i++){
		BasicBlock * B = I2->getIncomingBlock(i);
		if(phi1[B]==I2->getIncomingValue(i)){
			phi1.erase(B);
		}
	}

	return phi1.empty();
}

bool SIMDDeadlockEliminationNvidia::cleanRedundantPHIs(Function *F)
{
	bool found = true;
	for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I)
    {
       	BasicBlock * B = I;
   	    found &=cleanRedundantPHIs(B);
    }
	return found;
}

bool SIMDDeadlockEliminationNvidia::cleanRedundantPHIs(BasicBlock *B)
{
	bool found = true;
	std::unordered_set<Instruction *> * toRemoveInsts = new std::unordered_set<Instruction *>;
    for (BasicBlock::iterator II = B->begin(), IE = B->end(); II != IE; ++II)
    {
    	Instruction * Inst = II;
    	if(std::find(toRemoveInsts->begin(),toRemoveInsts->end(),Inst)!=toRemoveInsts->end()) continue;
    	if(dyn_cast<PHINode>(Inst)){
    		for (BasicBlock::iterator III = B->begin(), IIE = B->end(); III != IIE; ++III)
    	    {
    			if(Inst != III){
	   				Instruction * Instcmp = III;
	   				if(dyn_cast<PHINode>(Instcmp)){
	    				if(isIdenticalPHIs(dyn_cast<PHINode>(Inst),dyn_cast<PHINode>(Instcmp))){
							Instcmp->replaceAllUsesWith(Inst);
							toRemoveInsts->insert(Instcmp);
	    				}
	    			}
	    		}
    	    }
	    }
    }

    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I)
	{
    	bool allvaluesarethesame = false;
	    Value * v = nullptr;
	    if(dyn_cast<PHINode>(I)){
	    	allvaluesarethesame = true;
	    	PHINode * phI = dyn_cast<PHINode>(I);
	    	v = phI->getIncomingValue(0);
	   		for(unsigned i=0; i<phI->getNumIncomingValues();i++){
	   			if(phI->getIncomingValue(i)!=v){
	   				allvaluesarethesame=false;
	   			}
	    	}
		}
		if(allvaluesarethesame){
			//replace all uses of Instcmp with Inst
			I->replaceAllUsesWith(v);
			toRemoveInsts->insert(I);
		}
	}

    for (auto I = toRemoveInsts->begin(), IE = toRemoveInsts->end(); I != IE; ++I)
    {
		found = false;
	    Instruction * Inst = (*I);
	    if(Inst->hasNUsesOrMore(1)){
			for (auto it =Inst->user_begin(), et = Inst->user_end(); it != et; ++it)
			{
				Instruction * Iuser = dyn_cast<Instruction>(*it);
				Iuser->dump();
			}
			abort();
	    }

	    if(Inst->getParent()) //otherwise, it is already removed
	    	Inst->eraseFromParent();
    }

    return found;
}
