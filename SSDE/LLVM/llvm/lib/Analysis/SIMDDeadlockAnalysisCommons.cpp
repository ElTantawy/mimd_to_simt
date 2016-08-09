#include "llvm/Analysis/SIMDDeadlockAnalysisCommons.h"
#define NVIDIA_RECONVERGENCE
using namespace llvm;


SpanningTree::SpanningTree(Function *iF, DominatorTree *iDT)
{
	m_F = iF;
	m_DT = iDT;
	m_CFG = new Graph();
	m_SpanningTree = new Graph();
	//Create all nodes
	for(auto it=iF->begin(), et=iF->end(); it!=et; ++it){
		BasicBlock * BB = it;
		Node * n = new Node(BB);
		m_CFG->add_node(BB,n);
		m_SpanningTree->add_node(BB,n);
	}

	//Create all edges
	for(auto it=iF->begin(), et=iF->end(); it!=et; ++it){
		BasicBlock * src = it;
		if(dyn_cast<BranchInst>(src->getTerminator())){
			BranchInst * brInst = dyn_cast<BranchInst>(src->getTerminator());
			for(unsigned i=0; i<brInst->getNumSuccessors();i++){
				BasicBlock * trg = brInst->getSuccessor(i);
				Edge * e = new Edge(m_CFG->get_node(src),m_CFG->get_node(trg),brInst);
				m_CFG->add_edge(e);
			}
		}
	}

	STDFS(m_F->begin());
	calculate_st_ancestors();
	calculate_st_descendants();
	classifyEdges();
}

void SpanningTree::calculate_st_ancestors()
{
	for(auto it=m_SpanningTree->get_nodes()->begin(), et=m_SpanningTree->get_nodes()->end(); it!=et; ++it){
		Node * curNode = it->second;
		calculate_st_ancestors(curNode);
	}
}

void SpanningTree::calculate_st_descendants()
{
	for(auto it=m_SpanningTree->get_nodes()->begin(), et=m_SpanningTree->get_nodes()->end(); it!=et; ++it){
		Node * curNode = it->second;
		calculate_st_descendants(curNode);
	}
}

void SpanningTree::calculate_st_ancestors(Node *n)
{
	Node * curancestor = n;
	while(curancestor->getParent()!=nullptr){
		n->add_ancestor(curancestor->getParent());
		curancestor = curancestor->getParent();
	}
}

void SpanningTree::calculate_st_descendants(Node *n)
{
	unordered_set<Node *> * curproperdescendants = n->get_proper_descendants();
	for(auto it=curproperdescendants->begin(),et=curproperdescendants->end(); it!=et; ++it){
			Node * curNode = *it;
			n->add_descendant(curNode);
			calculate_st_descendants(n,curNode);
	}
}

void SpanningTree::calculate_st_descendants(Node *n, Node *curNode)
{
	unordered_set<Node *> * curproperdescendants = curNode->get_proper_descendants();
	for(auto it=curproperdescendants->begin(),et=curproperdescendants->end(); it!=et; ++it){
			Node * tcurNode = *it;
			n->add_descendant(tcurNode);
			calculate_st_descendants(n,tcurNode);
	}
}
void SpanningTree::STDFS(BasicBlock * curBB)
{
	unordered_set<BasicBlock *> * visitedNodes = new unordered_set<BasicBlock *>;
	visitedNodes->insert(curBB);
	if(dyn_cast<BranchInst>(curBB->getTerminator())){
		BranchInst * brInst = dyn_cast<BranchInst>(curBB->getTerminator());
		for(unsigned i=0; i<brInst->getNumSuccessors();i++){
			BasicBlock * succ = brInst->getSuccessor(i);
			if(visitedNodes->find(succ)==visitedNodes->end()){
				m_SpanningTree->add_edge(m_CFG->get_edge(curBB,succ));
				m_CFG->get_node(curBB)->add_outgoing_spt_edge(m_CFG->get_edge(curBB,succ));
				m_CFG->get_node(succ)->setParent(m_CFG->get_node(curBB));
				m_CFG->get_node(curBB)->add_proper_descendant(m_CFG->get_node(succ));
				STDFS(succ,visitedNodes);
			}
		}
	}

}


void SpanningTree::dump()
{
	errs() << "CFG Graph:\n";
	m_CFG->dump();
	errs() << "Spanning Tree Graph:\n";
	m_SpanningTree->dump();
}

void SpanningTree::STDFS(BasicBlock * curBB, unordered_set<BasicBlock *> * visitedNodes)
{
	visitedNodes->insert(curBB);
	if(dyn_cast<BranchInst>(curBB->getTerminator())){
		BranchInst * brInst = dyn_cast<BranchInst>(curBB->getTerminator());
		for(unsigned i=0; i<brInst->getNumSuccessors();i++){
			BasicBlock * succ = brInst->getSuccessor(i);
			if(visitedNodes->find(succ)==visitedNodes->end()){
				m_SpanningTree->add_edge(m_CFG->get_edge(curBB,succ));
				m_CFG->get_node(curBB)->add_outgoing_spt_edge(m_CFG->get_edge(curBB,succ));
				m_CFG->get_node(succ)->setParent(m_CFG->get_node(curBB));
				m_CFG->get_node(curBB)->add_proper_descendant(m_CFG->get_node(succ));
				STDFS(succ,visitedNodes);
			}
		}
	}
}

void SpanningTree::classifyEdges()
{
	for(auto it=m_CFG->get_edges()->begin(), et=m_CFG->get_edges()->end(); it!=et; ++it){
		Edge * curEdge = *it;
		Node * srcNode = curEdge->getSrc();
		Node * trgNode = curEdge->getTrg();
		if(m_SpanningTree->get_edges()->find(curEdge)!=m_SpanningTree->get_edges()->end()){
			curEdge->set_edge_type(Advancing);
			curEdge->set_advancing_edge_type(Tree);
			curEdge->set_retreating_edge_type(NARET);
		}else{
			if(trgNode->get_ancestors()->find(srcNode)!=trgNode->get_ancestors()->end()){
				curEdge->set_edge_type(Advancing);
				curEdge->set_advancing_edge_type(Forward);
				curEdge->set_retreating_edge_type(NARET);
			}else if((srcNode->get_ancestors()->find(trgNode)!=srcNode->get_ancestors()->end()) || (srcNode==trgNode)){
				curEdge->set_edge_type(Retreating);
				curEdge->set_advancing_edge_type(NAAET);
				if(m_DT->dominates(trgNode->getBB(),srcNode->getBB()) || (trgNode->getBB()==srcNode->getBB()) ){
					curEdge->set_retreating_edge_type(Backward);
				}else{
					curEdge->set_retreating_edge_type(NotBackward);
				}
			}else{
				curEdge->set_edge_type(Cross);
				curEdge->set_advancing_edge_type(NAAET);
				curEdge->set_retreating_edge_type(NARET);
			}
		}
	}
}

bool FunctionHelperInfo::doesPathContainBB(BasicBlock* B, unordered_set<BasicBlock*> *Path)
{
	if(find(Path->begin(),Path->end(),B)!=Path->end()) return true;
	return false;
}

void FunctionHelperInfo::getBackwardPath(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2)
{
	bool changed = true;
	unsigned size = 0;
	while(changed){
		size = (unsigned)P1to2->size();
		getBackwardPathBBs(B1,B2,P1to2);
		changed = (size!=(unsigned)P1to2->size());
	}

}

bool FunctionHelperInfo::getBackwardPathBBs(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2)
{
	unordered_set<BasicBlock*> *checked = new unordered_set<BasicBlock*>;
	checked->clear();
	checked->insert(B1);
	if(B1 == B2){
		if(P1to2->find(B1)==P1to2->end()){
			P1to2->insert(B1);
		}
		return true;
	}else{
		bool onPath=false;
		for (auto it = pred_begin(B1), et = pred_end(B1); it != et; ++it)
		{
			BasicBlock * P = (*it);
			if(checked->find(P)!=checked->end()){
				if(P1to2->find(P)!=P1to2->end()){
					onPath=true;
				}
				continue;
			}
			bool curonPath = getBackwardPathBBsRecursive(P,B2,P1to2,checked);
			onPath |= curonPath;
		}
		if(onPath)
		{
			if(P1to2->find(B1)==P1to2->end()){
				P1to2->insert(B1);
			}
			if(P1to2->find(B2)==P1to2->end()){
				P1to2->insert(B2);
			}
		}
		return onPath;
	}
}

bool FunctionHelperInfo::getBackwardPathBBsRecursive(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2,
													unordered_set<BasicBlock*> *checked)
{
	checked->insert(B1);
	if(B1 == B2){
		return true;
	}else{
		bool onPath=false;
		for (auto it = pred_begin(B1), et = pred_end(B1); it != et; ++it)
		{
			BasicBlock * P = (*it);
			if(checked->find(P)!=checked->end()){
				if(find(P1to2->begin(),P1to2->end(),P)!=P1to2->end()){
					onPath=true;
				}
				continue;
			}
			bool curonPath = getBackwardPathBBsRecursive(P,B2,P1to2,checked);
			onPath |= curonPath;
		}
		if(onPath){
			if(P1to2->find(B1)==P1to2->end()){
				P1to2->insert(B1);
			}
		}
		return onPath;
	}
}

void FunctionHelperInfo::print()
{
	for(auto it=m_CondBranches->begin(), ite=m_CondBranches->end(); it!=ite; ++it){
		BRExtendedInfo * BrInfo = *it;
		BrInfo->print();
	}
}

void FunctionHelperInfo::getPath(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2)
{
	bool changed = true;
	unsigned size = 0;
	while(changed){
		size = (unsigned)P1to2->size();
		getPathBBs(B1,B2,P1to2);
		changed = (size!=(unsigned)P1to2->size());
	}
}

bool FunctionHelperInfo::getPathBBs(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2)
{
	unordered_set<BasicBlock*> *checked = new unordered_set<BasicBlock*>;
	checked->clear();
	checked->insert(B1);
	if(B1 == B2){
		bool selfLoop = false;
		TerminatorInst *TI = B1->getTerminator();
		for(unsigned i=0; i<TI->getNumSuccessors();i++){
			BasicBlock * S = TI->getSuccessor(i);
			if(S==B2) selfLoop = true;
		}
		if(selfLoop){
			if(P1to2->find(B1)==P1to2->end()){
				P1to2->insert(B1);
			}
		}
		return selfLoop;
	}else{
		bool onPath=false;
		TerminatorInst *TI = B1->getTerminator();
		for(unsigned i=0; i<TI->getNumSuccessors();i++){
			BasicBlock * S = TI->getSuccessor(i);
			if(checked->find(S)!=checked->end()){
				if(P1to2->find(S)!=P1to2->end()){
					onPath=true;
				}
				continue;
			}			bool curonPath = getPathBBsRecursive(S,B2,P1to2,checked);
			onPath |= curonPath;
		}
		if(onPath)
		{
			if(P1to2->find(B1)==P1to2->end()){
				P1to2->insert(B1);
			}
			if(P1to2->find(B2)==P1to2->end()){
				P1to2->insert(B2);
			}
		}
		return onPath;
	}
}

bool FunctionHelperInfo::getPathBBsRecursive(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2,
											unordered_set<BasicBlock*> *checked)
{
	checked->insert(B1);
	if(B1 == B2){
		if(P1to2->find(B2)==P1to2->end()){
			P1to2->insert(B2);
		}
		return true;
	}else{
		bool onPath=false;
		TerminatorInst *TI = B1->getTerminator();
		for(unsigned i=0; i<TI->getNumSuccessors();i++){
			BasicBlock * S = TI->getSuccessor(i);
			if(checked->find(S)!=checked->end()){
				if(P1to2->find(S)!=P1to2->end()){
					onPath=true;
				}
				continue;
			}			bool curonPath = getPathBBsRecursive(S,B2,P1to2,checked);
			onPath |= curonPath;
		}
		if(onPath){
			if(P1to2->find(B1)==P1to2->end()){
				P1to2->insert(B1);
			}
		}
		return onPath;
	}
}

/*
 * Recursively track dependecies for each operand. If only dependencies with the loop is required, then the algorithm terminates operands are
 * outside the loop.
 */
void FunctionHelperInfo::trackOperandDependencies(Value* iOp, Value* Op, unordered_set<Value* > *DependencyLog, unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,unordered_set<Instruction*> * LoopInstructions,bool withinLoop)
{
	if(VisitedInstructionDependencies->find(Op)!=VisitedInstructionDependencies->end()) return;
	VisitedInstructionDependencies->insert(Op);

	if(withinLoop){
		if(dyn_cast<Instruction>(Op)){
			if(LoopInstructions->find(dyn_cast<Instruction>(Op))==LoopInstructions->end()){
				return;
			}
		}
	}

	if(DependencyLog->find(Op)==DependencyLog->end()){
		DependencyLog->insert(Op);
		extractDataDependencies(iOp,Op,DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
		extractControlDependencies(iOp,Op,DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
	}
	return;
}


void FunctionHelperInfo::extractControlDependencies(Value* iOp, Value *Op, unordered_set<Value* > *DependencyLog,unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,unordered_set<Instruction*> * LoopInstructions,bool withinLoop)
{
	if(!dyn_cast<Instruction>(Op)) return;
	assert(dyn_cast<Instruction>(iOp));
	Instruction *iI=dyn_cast<Instruction>(iOp);
	const BasicBlock * iBasicBlock = iI->getParent();
	Instruction *I=dyn_cast<Instruction>(Op);
	const BasicBlock * currBasicBlock = I->getParent();
	for (auto it = pred_begin(currBasicBlock), et = pred_end(currBasicBlock); it != et; ++it)
	{
	  const BasicBlock* predecessor = *it;
	  if (((!PDT->dominates(iBasicBlock,predecessor)) or (currBasicBlock==predecessor))){
		  if(dyn_cast<BranchInst>(predecessor->getTerminator())){
			  const BranchInst *BI = dyn_cast<BranchInst>(predecessor->getTerminator());
			  trackOperandDependencies(iOp,const_cast<Value *>((const Value *)BI),DependencyLog,VisitedInstructionDependencies,
					  	  	  	  	  VisitedControlDependencies,LoopInstructions,withinLoop);
		  }
	  }else if(currBasicBlock!=predecessor){
		  Value * newOp = (Value *)(const_cast<TerminatorInst *>(predecessor->getTerminator()));
		  if(VisitedControlDependencies->find(newOp)==VisitedControlDependencies->end()){
			  VisitedControlDependencies->insert(newOp);
			  extractControlDependencies(iOp,newOp,DependencyLog,VisitedInstructionDependencies,
					  	  	  	  	  	  VisitedControlDependencies,LoopInstructions,withinLoop);
	  	  }
	  }
	}
}

void FunctionHelperInfo::extractPHIDependencies(Value* iOp, BasicBlock *OrgPHIBB, BasicBlock *PHIBB,
												Value *Op, unordered_set<Value* > *DependencyLog,
												unordered_set<Value *> *VisitedInstructionDependencies,
												unordered_set<Value* > *VisitedControlDependencies,
												unordered_set<BasicBlock *> *VisitedPHIContributingNodes,
												unordered_set<Instruction*> * LoopInstructions,bool withinLoop)
{
	if(!dyn_cast<Instruction>(Op)) return;
	Instruction *I=dyn_cast<Instruction>(Op);
	const BasicBlock * currBasicBlock = I->getParent();
	VisitedPHIContributingNodes->insert(const_cast<BasicBlock *>(currBasicBlock));
	for (auto it = pred_begin(currBasicBlock), et = pred_end(currBasicBlock); it != et; ++it)
	{
		BasicBlock* predecessor = (const_cast<BasicBlock *> (*it));
		if(VisitedPHIContributingNodes->find(predecessor)!=VisitedPHIContributingNodes->end()) continue;
		  if (((!PDT->dominates(PHIBB,predecessor)) or (currBasicBlock==predecessor))){
			const BranchInst *BI = dyn_cast<BranchInst>(predecessor->getTerminator());
			trackOperandDependencies(iOp,const_cast<Value *>((const Value *)BI),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
		}else if((currBasicBlock!=predecessor) && (!DT->dominates(OrgPHIBB->begin(),predecessor->begin())) /*The second condition could be redundant*/ ){
			  Value * newOp = (Value *)(const_cast<TerminatorInst *>(predecessor->getTerminator()));
			  if(VisitedPHIContributingNodes->find(predecessor)==VisitedPHIContributingNodes->end()){
				  extractPHIDependencies(iOp,OrgPHIBB,PHIBB,newOp,DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,VisitedPHIContributingNodes,LoopInstructions,withinLoop);
		  	  }
		  }
	}
}

void FunctionHelperInfo::extractStoreDependencies(Value* iOp, BasicBlock *PHIBB, Value *Op,
												unordered_set<Value* > *DependencyLog,
												unordered_set<Value *> *VisitedInstructionDependencies,
												unordered_set<Value* > *VisitedControlDependencies,
												unordered_set<BasicBlock *> *VisitedPHIContributingNodes,
												unordered_set<Instruction*> * LoopInstructions,bool withinLoop)
{
	if(!dyn_cast<Instruction>(Op)) return;
	Instruction *I=dyn_cast<Instruction>(Op);
	const BasicBlock * currBasicBlock = I->getParent();
	VisitedPHIContributingNodes->insert(const_cast<BasicBlock *>(currBasicBlock));
	for (auto it = pred_begin(currBasicBlock), et = pred_end(currBasicBlock); it != et; ++it)
	{
		BasicBlock* predecessor = const_cast<BasicBlock *>(*it);
		if(VisitedPHIContributingNodes->find(predecessor)!=VisitedPHIContributingNodes->end()) continue;
		  if (((!PDT->dominates(PHIBB,predecessor)) or (currBasicBlock==predecessor))){
			  const Instruction *BI = dyn_cast<Instruction>(predecessor->getTerminator());
			trackOperandDependencies(iOp,const_cast<Value *>((const Value *)BI),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
		}else if((currBasicBlock!=predecessor)){
			  Value * newOp = (Value *)(const_cast<TerminatorInst *>(predecessor->getTerminator()));
			  if(VisitedPHIContributingNodes->find(predecessor)==VisitedPHIContributingNodes->end()){
				  extractStoreDependencies(iOp,PHIBB,newOp,DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,VisitedPHIContributingNodes,LoopInstructions,withinLoop);
		  	  }
		  }
	}
}

void FunctionHelperInfo::extractDataDependenciesForCallInst(Value* iOp, Value *Op, unordered_set<Value* > *DependencyLog,
															unordered_set<Value *> *VisitedInstructionDependencies,
															unordered_set<Value* > *VisitedControlDependencies,
															unordered_set<Instruction*> * LoopInstructions,bool withinLoop)
{
	CallInst * cI = dyn_cast<CallInst>(Op);
	if(cI->isInlineAsm() || cI->isThreadIDCall()){
		DependencyLog->insert(Op);
	}else if(cI->isIntrinsicCall() || cI->isMalloc()){
		unsigned numArgs=cI->getNumArgOperands();
		for(unsigned i=0; i<numArgs; i++){
			trackOperandDependencies(iOp,cI->getArgOperand(i),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			if(cI->getArgOperand(i)->getType()->isPointerTy()){
				extractDataDependenciesForAllocInst(iOp,cI->getArgOperand(i),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			}
		}
	}else{
		//abort(); uncomment if you want to make sure all functions are inlined
		unsigned numArgs=cI->getNumArgOperands();
		for(unsigned i=0; i<numArgs; i++){
			trackOperandDependencies(iOp,cI->getArgOperand(i),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			if(cI->getArgOperand(i)->getType()->isPointerTy()){
				extractDataDependenciesForAllocInst(iOp,cI->getArgOperand(i),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			}
		}
	}
}



void FunctionHelperInfo::extractDataDependenciesForAllocInst(Value* iOp, Value * Op, unordered_set<Value* > *DependencyLog,
															unordered_set<Value *> *VisitedInstructionDependencies,
															unordered_set<Value* > *VisitedControlDependencies,
															unordered_set<Instruction*> * LoopInstructions,bool withinLoop)
{
	//AllocaInst *Ia = dyn_cast<AllocaInst>(I);
	for (auto it = Op->user_begin(), et = Op->user_end(); it != et; ++it)
	{
		if(dyn_cast<Instruction>(*it)){
			Instruction * Iuser = dyn_cast<Instruction>(*it);
			if(dyn_cast<StoreInst>(Iuser)){

				if(withinLoop){
					if(LoopInstructions->find(Iuser)==LoopInstructions->end()){
						continue;
					}
				}

				if(DependencyLog->find(Iuser)==DependencyLog->end()){
					DependencyLog->insert(Iuser);
					trackOperandDependencies(iOp,Iuser->getOperand(0),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
					for (auto it = pred_begin(Iuser->getParent()), et = pred_end(Iuser->getParent()); it != et; ++it)
					{
						BasicBlock * pred = (*it);
						unordered_set<BasicBlock*> * PHIContributingNodes = new unordered_set<BasicBlock*>;
						trackOperandDependencies(iOp,pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
						extractStoreDependencies(iOp,Iuser->getParent(),pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,PHIContributingNodes,LoopInstructions,withinLoop);
					}
				}
			}else if(dyn_cast<AtomicRMWInst>(Iuser)){
				if(DependencyLog->find(Iuser)==DependencyLog->end()){
					DependencyLog->insert(Iuser);
					trackOperandDependencies(iOp,Iuser->getOperand(1),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
					for (auto it = pred_begin(Iuser->getParent()), et = pred_end(Iuser->getParent()); it != et; ++it)
					{
						BasicBlock * pred = (*it);
						unordered_set<BasicBlock*> * PHIContributingNodes = new unordered_set<BasicBlock*>;
						trackOperandDependencies(iOp,pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
						extractStoreDependencies(iOp,Iuser->getParent(),pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,PHIContributingNodes,LoopInstructions,withinLoop);
					}
				}
			}else if(dyn_cast<AtomicCmpXchgInst>(Iuser)){
				if(DependencyLog->find(Iuser)==DependencyLog->end()){
					DependencyLog->insert(Iuser);
					trackOperandDependencies(iOp,Iuser->getOperand(1),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
					for (auto it = pred_begin(Iuser->getParent()), et = pred_end(Iuser->getParent()); it != et; ++it)
					{
						BasicBlock * pred = (*it);
						unordered_set<BasicBlock*> * PHIContributingNodes = new unordered_set<BasicBlock*>;
						trackOperandDependencies(iOp,pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
						extractStoreDependencies(iOp,Iuser->getParent(),pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,PHIContributingNodes,LoopInstructions,withinLoop);
					}
				}
			}else if(dyn_cast<CallInst>(Iuser)){
				if(DependencyLog->find(Iuser)==DependencyLog->end()){
					DependencyLog->insert(Iuser);
					if(dyn_cast<CallInst>(Iuser)->isMemCpy()){
						trackOperandDependencies(iOp,Iuser->getOperand(1),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
						for (auto it = pred_begin(Iuser->getParent()), et = pred_end(Iuser->getParent()); it != et; ++it)
						{
							BasicBlock * pred = (*it);
							unordered_set<BasicBlock*> * PHIContributingNodes = new unordered_set<BasicBlock*>;
							trackOperandDependencies(iOp,pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
							extractStoreDependencies(iOp,Iuser->getParent(),pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,PHIContributingNodes,LoopInstructions,withinLoop);
						}
					}else if(dyn_cast<CallInst>(Iuser)->isAtomicCall()){
						trackOperandDependencies(iOp,Iuser->getOperand(0),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
						for (auto it = pred_begin(Iuser->getParent()), et = pred_end(Iuser->getParent()); it != et; ++it)
						{
							BasicBlock * pred = (*it);
							unordered_set<BasicBlock*> * PHIContributingNodes = new unordered_set<BasicBlock*>;
							trackOperandDependencies(iOp,pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
							extractStoreDependencies(iOp,Iuser->getParent(),pred->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,PHIContributingNodes,LoopInstructions,withinLoop);
						}
						if(dyn_cast<CallInst>(Iuser)->isAtomicLoadInc()){
							//It is just an increment no dependency on other variables
						}else{
							abort();
						}
					}
				}
			}
		}
	}
	return;
}

void FunctionHelperInfo::extractDataDependencies(Value* iOp, Value* Op, unordered_set<Value* > *DependencyLog,
												unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,
												unordered_set<Instruction*> * LoopInstructions,bool withinLoop)
{
	if(!dyn_cast<Instruction>(Op)) return;
	Instruction *I=dyn_cast<Instruction>(Op);
	unsigned numOps =I->getNumOperands();
	if(dyn_cast<CallInst>(I)){
		extractDataDependenciesForCallInst(iOp,Op,DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
	}else if(dyn_cast<AllocaInst>(I)){
		extractDataDependenciesForAllocInst(iOp,Op,DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
	}else if(dyn_cast<BranchInst>(I)){
		BranchInst * BI = dyn_cast<BranchInst>(I);
		if(BI->isConditional()){
			trackOperandDependencies(iOp,BI->getOperand(0),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
		}
	}else if(dyn_cast<PHINode>(I)){
		for(unsigned i=0; i<numOps; i++){
			trackOperandDependencies(iOp,I->getOperand(i),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			if(I->getOperand(i)->getType()->isPointerTy()){
				extractDataDependenciesForAllocInst(iOp,I->getOperand(i),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			}
			trackOperandDependencies(iOp,dyn_cast<PHINode>(I)->getIncomingBlock(i)->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			unordered_set<BasicBlock*> * PHIContributingNodes = new unordered_set<BasicBlock*>;
			extractPHIDependencies(iOp,I->getParent(),dyn_cast<PHINode>(I)->getIncomingBlock(i),dyn_cast<PHINode>(I)->getIncomingBlock(i)->getTerminator(),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,PHIContributingNodes,LoopInstructions,withinLoop);
		}
	}else{
		for(unsigned i=0; i<numOps; i++){
			trackOperandDependencies(iOp,I->getOperand(i),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			if(I->getOperand(i)->getType()->isPointerTy()){
				extractDataDependenciesForAllocInst(iOp,I->getOperand(i),DependencyLog,VisitedInstructionDependencies,VisitedControlDependencies,LoopInstructions,withinLoop);
			}
		}
	}
}

BRExtendedInfo *FunctionHelperInfo::getBrInfo(BranchInst *BrInst)
{
	for(auto it=m_CondBranches->begin(), ite=m_CondBranches->end(); it!=ite; ++it){
		BRExtendedInfo * BrInfo = *it;
		if(BrInfo->getBrInst()==BrInst){
			return BrInfo;
		}
	}
	return nullptr;
}

void BRExtendedInfo::print()
{
	errs() << "Branch Inst: \n";
	errs() << m_BrInst->getParent()->getName() << "\n";
	m_BrInst->dump();
	m_StackBasedRec->begin()->dump();
	m_iReqPDOMInstRelaxed->dump();
	m_finReqPDOMInstRelaxed->dump();

}

void BRExtendedInfo::calculateTruePath()
{
	#ifdef NVIDIA_RECONVERGENCE
	getPath(m_T, m_NvidiaBasedRec, m_PTtoR);
	for (auto it = m_PTtoR->begin() ; it != m_PTtoR->end(); ++it){
		if(*it == m_NvidiaBasedRec){
			m_PTtoR->erase(it);
			break;
		}
    }
	#else
	getPath(m_T, m_StackBasedRec, m_PTtoR);
	for (auto it = m_PTtoR->begin() ; it != m_PTtoR->end(); ++it){
		if(*it == m_StackBasedRec){
			m_PTtoR->erase(it);
			break;
		}
    }
	#endif
}

void BRExtendedInfo::calculateFalsePath()
{
	#ifdef NVIDIA_RECONVERGENCE
	getPath(m_F, m_NvidiaBasedRec, m_PFtoR);
	for (auto it = m_PFtoR->begin() ; it != m_PFtoR->end(); ++it){
		if(*it == m_NvidiaBasedRec){
			m_PFtoR->erase(it);
			break;
		}
    }
	#else
	getPath(m_F, m_StackBasedRec, m_PFtoR);
	for (auto it = m_PFtoR->begin() ; it != m_PFtoR->end(); ++it){
		if(*it == m_StackBasedRec){
			m_PFtoR->erase(it);
			break;
		}
	}
	#endif
}

void BackwardBranchInfo::calculateLoopInstrs()
{
	for (auto itBB = m_LoopBBs->begin(), etBB = m_LoopBBs->end(); itBB != etBB; ++itBB){
		for(auto itI = (*itBB)->begin(), etI = (*itBB)->end(); itI != etI; ++itI){
			m_LoopInstructions->insert(itI);
		}
	}
}

/* This perform the backward slicing by getting the dependencies of all loop exists.
 * Dependencies are stored in m_DependencyLog.
 * Also, dependencies within the loop are stored in m_DependencyLogWithinLoop which is a subset
 * of m_DependencyLog.
 * TODO: merge them to be computed in one pass.
 */

void BackwardBranchInfo::calculateDependencyLog()
{
	unordered_set<Value *> *VisitedInstructionDependencies = new unordered_set<Value *>;
	unordered_set<Value *> *VisitedControlDependencies = new unordered_set<Value *>;
	VisitedInstructionDependencies->clear();
	VisitedControlDependencies->clear();
	for(auto it = m_LoopExitConditions->begin(), et = m_LoopExitConditions->end(); it!=et; ++it){
		TerminatorInst *exitBranch = (*it);
		trackOperandDependencies((Value*)exitBranch,(Value*)exitBranch,m_DependencyLog,VisitedInstructionDependencies,
								VisitedControlDependencies,m_LoopInstructions,false);
	}
	VisitedInstructionDependencies->clear();
	VisitedControlDependencies->clear();
	for(auto it = m_LoopExitConditions->begin(), et = m_LoopExitConditions->end(); it!=et; ++it){
		TerminatorInst *exitBranch = (*it);
		trackOperandDependencies((Value*)exitBranch,(Value*)exitBranch,m_DependencyLogWithinLoop,VisitedInstructionDependencies,
								VisitedControlDependencies,m_LoopInstructions,true);
	}

}

bool BackwardBranchInfo::isdependenton(Value * Op)
{
	if(m_DependencyLog->find(Op)!=m_DependencyLog->end()) return true;
	return false;
}

bool BackwardBranchInfo::isdependentonWithinLoop(Value * Op)
{
	if(m_DependencyLogWithinLoop->find(Op)!=m_DependencyLogWithinLoop->end()) return true;
	return false;
}

/*
 * Straight Forward: Checks for (global or shared) load instructions within the loop body.
 * Note: the code below assumes that all generic memory references are lowered to device
 * specific memory spaces (i.e., lowering memory spaces passes were invoked earlier).
 */

void BackwardBranchInfo::calculateSharedLoadInstSet()
{
	for(auto itI = m_LoopInstructions->begin(), itIe = m_LoopInstructions->end(); itI!=itIe; ++itI){
		if(dyn_cast<LoadInst>((*itI)) || dyn_cast<AtomicRMWInst>((*itI)) || dyn_cast<AtomicCmpXchgInst>((*itI)))
		{
			unsigned addrspace = dyn_cast<LoadInst>((*itI))          ? dyn_cast<LoadInst>((*itI))->getPointerAddressSpace():
						dyn_cast<AtomicRMWInst>((*itI))     ? dyn_cast<AtomicRMWInst>((*itI))->getPointerAddressSpace():
						dyn_cast<AtomicCmpXchgInst>((*itI)) ? dyn_cast<AtomicCmpXchgInst>((*itI))->getPointerAddressSpace(): (unsigned)-1;

			assert(m_SharedLoadInstSet->find(*itI)==m_SharedLoadInstSet->end());
			if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
				if(isdependentonWithinLoop(*itI)){
					m_SharedLoadInstSet->insert((*itI));
				}
			}
		}

		if(dyn_cast<CallInst>((*itI))){
			CallInst * cI = dyn_cast<CallInst>((*itI));
			if(!cI->isInlineAsm() && cI->getCalledFunction()){
				if(cI->isMemCpy()){
					unsigned addrspace = cI->getArgOperand(1)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						if(isdependentonWithinLoop(*itI)){
							m_SharedLoadInstSet->insert((*itI));
						}
					}
				}else if(cI->isAtomicCall()){
					//e.g., llvm.nvvm.load.inc
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						if(isdependentonWithinLoop(*itI)){
							m_SharedLoadInstSet->insert((*itI));
						}
					}
				}
			}
		}
	}
}

void BackwardBranchInfo::calculateSharedStoreInstSetWithinLoop()
{
	for(auto itI = m_LoopInstructions->begin(), itIe = m_LoopInstructions->end(); itI!=itIe; ++itI){
		if(dyn_cast<StoreInst>(*itI) || dyn_cast<AtomicRMWInst>(*itI) || dyn_cast<AtomicCmpXchgInst>(*itI))
		{
			unsigned addrspace = dyn_cast<StoreInst>(*itI)          ? dyn_cast<StoreInst>(*itI)->getPointerAddressSpace():
						dyn_cast<AtomicRMWInst>(*itI)     ? dyn_cast<AtomicRMWInst>(*itI)->getPointerAddressSpace():
						dyn_cast<AtomicCmpXchgInst>(*itI) ? dyn_cast<AtomicCmpXchgInst>(*itI)->getPointerAddressSpace(): (unsigned)-1;

			assert(m_SharedStoreInstSetWithinLoop->find(*itI)==m_SharedStoreInstSetWithinLoop->end());
			if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
				m_SharedStoreInstSetWithinLoop->insert(*itI);
			}
		}

		if(dyn_cast<CallInst>(*itI)){
			CallInst * cI = dyn_cast<CallInst>(*itI);
			if(!cI->isInlineAsm() && cI->getCalledFunction()){
				if(cI->isMemCpy()){
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						m_SharedStoreInstSetWithinLoop->insert((*itI));
					}
				}else if(cI->isAtomicCall()){
					//e.g., llvm.nvvm.load.inc
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						m_SharedStoreInstSetWithinLoop->insert((*itI));
					}
				}
			}
		}

	}

}

/*
 * Relaxed ReqPDOM can be used with AWARE since aware relies on TimedOut reconvergence as a back-up mechanism. It is safe
 * to relax the ReqPDOM estimation avoiding overly conservative decisions. For example, Shrdwrites condiser only reachable
 * stores since it is communication with parallel paths is less common in pratice.
 */
void BackwardBranchInfo::calculateSharedStoreInstSetRelaxed()
{
	for(auto itI = m_reachableInstructions->begin(), itIe = m_reachableInstructions->end(); itI!=itIe; ++itI){
		if(dyn_cast<StoreInst>(itI->first) || dyn_cast<AtomicRMWInst>(itI->first) || dyn_cast<AtomicCmpXchgInst>(itI->first))
		{
			unsigned addrspace = dyn_cast<StoreInst>(itI->first)          ? dyn_cast<StoreInst>(itI->first)->getPointerAddressSpace():
						dyn_cast<AtomicRMWInst>(itI->first)     ? dyn_cast<AtomicRMWInst>(itI->first)->getPointerAddressSpace():
						dyn_cast<AtomicCmpXchgInst>(itI->first) ? dyn_cast<AtomicCmpXchgInst>(itI->first)->getPointerAddressSpace(): (unsigned)-1;

			assert(m_SharedStoreInstSetRelaxed->find(itI->first)==m_SharedStoreInstSetRelaxed->end());
			if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
				m_SharedStoreInstSetRelaxed->insert(itI->first);
			}
		}

		if(dyn_cast<CallInst>(itI->first)){
			CallInst * cI = dyn_cast<CallInst>(itI->first);
			if(!cI->isInlineAsm() && cI->getCalledFunction()){
				if(cI->isMemCpy()){
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						m_SharedStoreInstSetRelaxed->insert((itI->first));
					}
				}else if(cI->isAtomicCall()){
					//e.g., llvm.nvvm.load.inc
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						m_SharedStoreInstSetRelaxed->insert((itI->first));
					}
				}
			}
		}

	}

}

/*
 * Straight Forward: Checks for (global or shared) store instructions that are parallel to or reachable from
 * the loop that is created by this backward branch.  * Note: the code below assumes that all generic memory
 * references are lowered to device specific memory spaces (i.e., lowering memory spaces passes were invoked earlier).
 */

void BackwardBranchInfo::calculateSharedStoreInstSet()
{
	for(auto itI = m_reachableInstructions->begin(), itIe = m_reachableInstructions->end(); itI!=itIe; ++itI){
		if(dyn_cast<StoreInst>(itI->first) || dyn_cast<AtomicRMWInst>(itI->first) || dyn_cast<AtomicCmpXchgInst>(itI->first))
		{
			unsigned addrspace = dyn_cast<StoreInst>(itI->first)          ? dyn_cast<StoreInst>(itI->first)->getPointerAddressSpace():
						dyn_cast<AtomicRMWInst>(itI->first)     ? dyn_cast<AtomicRMWInst>(itI->first)->getPointerAddressSpace():
						dyn_cast<AtomicCmpXchgInst>(itI->first) ? dyn_cast<AtomicCmpXchgInst>(itI->first)->getPointerAddressSpace(): (unsigned)-1;

			assert(m_SharedStoreInstSet->find(itI->first)==m_SharedStoreInstSet->end());
			if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
				m_SharedStoreInstSet->insert(itI->first);
			}
		}

		if(dyn_cast<CallInst>(itI->first)){
			CallInst * cI = dyn_cast<CallInst>(itI->first);
			if(!cI->isInlineAsm() && cI->getCalledFunction()){
				if(cI->isMemCpy()){
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						m_SharedStoreInstSet->insert((itI->first));
					}
				}else if(cI->isAtomicCall()){
					//e.g., llvm.nvvm.load.inc
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						m_SharedStoreInstSet->insert((itI->first));
					}
				}
			}
		}

	}

	for(auto itI = m_parallelInstructions->begin(), itIe = m_parallelInstructions->end(); itI!=itIe; ++itI){
		if(dyn_cast<StoreInst>(itI->first) || dyn_cast<AtomicRMWInst>(itI->first) || dyn_cast<AtomicCmpXchgInst>(itI->first))
		{
			unsigned addrspace = dyn_cast<StoreInst>(itI->first)? dyn_cast<StoreInst>(itI->first)->getPointerAddressSpace():
						dyn_cast<AtomicRMWInst>(itI->first)? dyn_cast<AtomicRMWInst>(itI->first)->getPointerAddressSpace():
						dyn_cast<AtomicCmpXchgInst>(itI->first)? dyn_cast<AtomicCmpXchgInst>(itI->first)->getPointerAddressSpace(): (unsigned)-1;

			if(m_SharedStoreInstSet->find(itI->first)==m_SharedStoreInstSet->end()){
				if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
					m_SharedStoreInstSet->insert(itI->first);
				}
			}
		}

		if(dyn_cast<CallInst>(itI->first)){
			CallInst * cI = dyn_cast<CallInst>(itI->first);
			if(!cI->isInlineAsm() && cI->getCalledFunction()){
				if(cI->isMemCpy()){
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						m_SharedStoreInstSet->insert((itI->first));
					}
				}else if(cI->isAtomicCall()){
					//e.g., llvm.nvvm.load.inc
					unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						m_SharedStoreInstSet->insert((itI->first));
					}
				}
			}
		}
	}


}

void BackwardBranchInfo::calculateRedefiningSharedStoreInstSetWithinLoop()
{
	for(auto itIL = m_SharedLoadInstSet->begin(), itILe = m_SharedLoadInstSet->end(); itIL!=itILe; ++itIL){
		for(auto itIS = m_SharedStoreInstSetWithinLoop->begin(), itISe = m_SharedStoreInstSetWithinLoop->end(); itIS!=itISe; ++itIS){
			if(checkIfLoadAliasesWithStore((*itIL),(*itIS))){
				m_PotentialLoadStorePairsWithinLoop->push_back(make_pair((*itIS),(*itIL)));
				if(m_PotentialRedefiningStoresSetWithinLoop->find(*itIS)==m_PotentialRedefiningStoresSetWithinLoop->end()){
					m_PotentialRedefiningStoresSetWithinLoop->insert((*itIS));
				}
			}
		}
	}
}

/*
 * Straight Forward: all shared stores that may alias with a shared load is added to a potential redefining stores set
 */

void BackwardBranchInfo::calculateRedefiningSharedStoreInstSet()
{
	for(auto itIL = m_SharedLoadInstSet->begin(), itILe = m_SharedLoadInstSet->end(); itIL!=itILe; ++itIL){
		for(auto itIS = m_SharedStoreInstSet->begin(), itISe = m_SharedStoreInstSet->end(); itIS!=itISe; ++itIS){
			if(checkIfLoadAliasesWithStore((*itIL),(*itIS))){
				m_PotentialLoadStorePairs->push_back(make_pair((*itIS),(*itIL)));
				if(m_PotentialRedefiningStoresSet->find(*itIS)==m_PotentialRedefiningStoresSet->end()){
					m_PotentialRedefiningStoresSet->insert((*itIS));
				}
			}
		}
	}
}


/*
 * Relaxed ReqPDOM can be used with AWARE since aware relies on TimedOut reconvergence as a back-up mechanism. It is safe
 * to relax the ReqPDOM estimation avoiding overly conservative decisions. For example, the code below does not consider
 * a shared store that dominates a shared load to be a redefining store (as it is not common in codes to have a "lock release"
 * for example dominating a "loack acquire".
 */
void BackwardBranchInfo::calculateRedefiningSharedStoreInstSetRelaxed()
{
	for(auto itIL = m_SharedLoadInstSet->begin(), itILe = m_SharedLoadInstSet->end(); itIL!=itILe; ++itIL){
		for(auto itIS = m_SharedStoreInstSetRelaxed->begin(), itISe = m_SharedStoreInstSetRelaxed->end(); itIS!=itISe; ++itIS){
			if((*itIL!=*itIS) && !DT->dominates((*itIS),(*itIL))){
				if(checkIfLoadAliasesWithStore((*itIL),(*itIS))){
					m_PotentialLoadStorePairsRelaxed->push_back(make_pair((*itIS),(*itIL)));
					if(m_PotentialRedefiningStoresSetRelaxed->find(*itIS)==m_PotentialRedefiningStoresSetRelaxed->end()){
						m_PotentialRedefiningStoresSetRelaxed->insert((*itIS));
					}
				}
			}
		}
	}
}

void BackwardBranchInfo::calculateLoopExitNodes()
{
	for(auto itIL = m_LoopBBs->begin(), itILe = m_LoopBBs->end(); itIL!=itILe; ++itIL){
		BasicBlock * curBB = *itIL;
		TerminatorInst * TInst = curBB->getTerminator();
		for(unsigned i=0; i<TInst->getNumSuccessors();i++){
			BasicBlock * SuccBB = TInst->getSuccessor(i);
			if(!doesPathContainBB(SuccBB, m_LoopBBs)){
				m_LoopExitConditions->insert(TInst);
				m_LoopExitNodes->insert(SuccBB);
			}
		}
	}
}


void BackwardBranchInfo::calculateLoopBBs()
{
	if(isBackwardEdge(m_Self, m_TakenTarget)){
		getBackwardPath(m_Self,m_TakenTarget,m_LoopBBs);

		if(m_LoopBBs->begin()==m_LoopBBs->end())
		{
			m_createsLoop = false;
		}else{
			m_createsLoop = true;
		}
	}else{
		/*
		 * To detect unstructured loops, it follows:
		 * in Vugranam C. Sreedhar, Guang R. Gao, and Yong-Fong Lee. 1996.
		 * Identifying loops using DJ graphs. ACM Trans. Program. Lang. Syst. 18, 6
		 * (November 1996), 649-658.
		 */
		unsigned level = getDominanceLevel(m_TakenTarget);
		for (scc_iterator<Function *> I = scc_begin(m_Function),
		                              IE = scc_end(m_Function);
		                              I != IE; ++I) {
		  // Obtain the vector of BBs in this SCC and add them to the loop.
		  const vector<BasicBlock *> &SCCBBs = *I;
		  if(find(SCCBBs.begin(),SCCBBs.end(),m_TakenTarget)!=SCCBBs.end()){
			  for (vector<BasicBlock *>::const_iterator BBI = SCCBBs.begin(),
		                                                 BBIE = SCCBBs.end();
		                                                 BBI != BBIE; ++BBI) {
				  BasicBlock * BB = (*BBI);
				  if(getDominanceLevel(BB) >= level){
					  m_LoopBBs->insert(BB);
				  }
			  }
		  }
		}
	}
}

bool BackwardBranchInfo::checkIfVisited(BasicBlock *SuccBB,unordered_set<BRExtendedInfo *> * ReachBrSet,map<BasicBlock *,
										unordered_set<BRExtendedInfo *> *> * Visited)
{
	if(Visited->find(SuccBB)==Visited->end()){
		return false;
	}

	for(auto it = ReachBrSet->begin(), eit = ReachBrSet->end(); it!=eit; ++it){
		if(Visited->at(SuccBB)->find(*it)==Visited->at(SuccBB)->end()){
			return false;
		}
	}

	return true;
}

void BackwardBranchInfo::calculateReachableInstrs(BasicBlock *SuccBB,unordered_set<BRExtendedInfo *> * ReachBrSet,
													map<BasicBlock *,unordered_set<BRExtendedInfo *> *> * Visited)
{
	if(Visited->find(SuccBB)==Visited->end()){
		(*Visited)[SuccBB] = new unordered_set<BRExtendedInfo *>;
	}

	for(auto it = ReachBrSet->begin(), eit = ReachBrSet->end(); it!=eit; ++it){
		if(Visited->at(SuccBB)->find(*it)==Visited->at(SuccBB)->end()){
			Visited->at(SuccBB)->insert(*it);
		}
	}

	//include all instructions in the reconvergence BB
	for(auto itI = SuccBB->begin(), etI = SuccBB->end(); itI != etI; ++itI){
		if(dyn_cast<CallInst>(itI)){
			CallInst * cI = dyn_cast<CallInst>(itI);
			if(!cI->isInlineAsm() && cI->getCalledFunction()){
				if(cI->isBarrierCall()) return;
			}
		}else if(dyn_cast<BranchInst>(itI)){
			BranchInst * bI = dyn_cast<BranchInst>(itI);
			for(unsigned i=0; i<bI->getNumSuccessors();i++){
				/*
				 * This is specific to OpenMP and how current support deals with barriers.
				 * Barries at any locations are replaced by a branch instruction to single
				 * barrier at the end of the kernel.
				 */
				if(bI->getSuccessor(i)->getName().compare(".sync.and.next.state")==0){
					return;
				}
			}
		}
		if(m_reachableInstructions->find(itI)==m_reachableInstructions->end()){
			(*m_reachableInstructions)[itI] = new unordered_set<BRExtendedInfo *>;
		}
		for(auto it = ReachBrSet->begin(), eit = ReachBrSet->end(); it!=eit; ++it){
			if(m_reachableInstructions->at(itI)->find(*it)==m_reachableInstructions->at(itI)->end()){
				m_reachableInstructions->at(itI)->insert(*it);
			}
		}
	}

	TerminatorInst * TInst = SuccBB->getTerminator();
	if(dyn_cast<BranchInst>(TInst)){
		BranchInst * BrInst = dyn_cast<BranchInst>(TInst);
		BRExtendedInfo * BrInstInfo =getBrInfo(BrInst);
		if(BrInst->isConditional()){
			if(find(ReachBrSet->begin(),ReachBrSet->end(),BrInstInfo)==ReachBrSet->end()){
				ReachBrSet->insert(BrInstInfo);
			}
			for(unsigned i=0; i<TInst->getNumSuccessors(); i++){
				BasicBlock * SuccBB = TInst->getSuccessor(i);
				if(checkIfVisited(SuccBB,ReachBrSet,Visited)) continue;
				unordered_set<BRExtendedInfo *> * cursReachBrSet = new unordered_set<BRExtendedInfo *>;
				cursReachBrSet->clear();
				for(auto it = ReachBrSet->begin(), eit = ReachBrSet->end(); it!=eit; ++it){
					cursReachBrSet->insert((*it));
				}
				calculateReachableInstrs(SuccBB,cursReachBrSet,Visited);
			}
		}else{
			assert(TInst->getNumSuccessors()==1);
			BasicBlock * SuccBB = TInst->getSuccessor(0);
			if(!checkIfVisited(SuccBB,ReachBrSet,Visited)){
				unordered_set<BRExtendedInfo *> * cursReachBrSet = new unordered_set<BRExtendedInfo *>;
				cursReachBrSet->clear();
				for(auto it = ReachBrSet->begin(), eit = ReachBrSet->end(); it!=eit; ++it){
					cursReachBrSet->insert((*it));
				}

				calculateReachableInstrs(SuccBB,cursReachBrSet,Visited);
			}
		}
	}
}


/*
 * Please, refer to the definition of reachable instructions in Listing-1 in the paper.
 * Reachable paths are terminated upon encountering barriers. The m_reachableInstructions
 * includes all reachanble instructions along with a set of branch instructions that are
 * on the way to this reachable instruction.
 */
void BackwardBranchInfo::calculateReachableInstrs()
{
	map<BasicBlock *,unordered_set<BRExtendedInfo *> *> * Visited = new map<BasicBlock *,unordered_set<BRExtendedInfo *> *>;
	unordered_set<BRExtendedInfo *> * ReachBrSet = new unordered_set<BRExtendedInfo *>;
	ReachBrSet->clear();
	/*
	 * TODO: it is sufficient to consider reachable instructions from loop exits reconvergence point
	 * the code considers reachable instructions from each loop exit which overlaps with parallel
	 * instructions (if the reconvergence point of the loop was pre-computed, one could have used it
	 * directly).
	*/
	for(auto itIL = m_LoopExitNodes->begin(), itILe = m_LoopExitNodes->end(); itIL!=itILe; ++itIL){
		BasicBlock * curExitBB = *itIL;
		(*Visited)[curExitBB]= new unordered_set<BRExtendedInfo *>;
		//include all instructions in the exit BB
		for(auto itI = curExitBB->begin(), etI = curExitBB->end(); itI != etI; ++itI){
			if(dyn_cast<CallInst>(itI)){
				CallInst * cI = dyn_cast<CallInst>(itI);
				if(!cI->isInlineAsm() && cI->getCalledFunction()){
					if(cI->isBarrierCall()) return;
				}
			}else if(dyn_cast<BranchInst>(itI)){
				BranchInst * bI = dyn_cast<BranchInst>(itI);
				for(unsigned i=0; i<bI->getNumSuccessors();i++){
					/*
					 * This is specific to OpenMP and how current support deals with barriers.
					 * Barries at any locations are replaced by a branch instruction to single
					 * barrier at the end of the kernel.
					 */
					if(bI->getSuccessor(i)->getName().compare(".sync.and.next.state")==0){
						return;
					}
				}
			}

			if(m_reachableInstructions->find(itI)==m_reachableInstructions->end()){
				(*m_reachableInstructions)[itI] = new unordered_set<BRExtendedInfo *>;
			}
			for(auto it = ReachBrSet->begin(), eit = ReachBrSet->end(); it!=eit; ++it){
				if(find(m_reachableInstructions->at(itI)->begin(),m_reachableInstructions->at(itI)->end(),*it)==m_reachableInstructions->at(itI)->end()){
					m_reachableInstructions->at(itI)->insert(*it);
				}
			}
		}

		TerminatorInst * TInst = curExitBB->getTerminator();
		if(dyn_cast<BranchInst>(TInst)){
			BranchInst * BrInst = dyn_cast<BranchInst>(TInst);
			BRExtendedInfo * BrInstInfo =getBrInfo(BrInst);
			if(BrInst->isConditional()){
				if(find(ReachBrSet->begin(),ReachBrSet->end(),BrInstInfo)==ReachBrSet->end()){
					ReachBrSet->insert(BrInstInfo);
				}
			}
			for(unsigned i=0; i<TInst->getNumSuccessors(); i++){
				BasicBlock * SuccBB = TInst->getSuccessor(i);
				if(checkIfVisited(SuccBB,ReachBrSet,Visited)) continue;
				unordered_set<BRExtendedInfo *> * cursReachBrSet = new unordered_set<BRExtendedInfo *>;
				cursReachBrSet->clear();
				for(auto it = ReachBrSet->begin(), eit = ReachBrSet->end(); it!=eit; ++it){
					cursReachBrSet->insert((*it));
				}
				calculateReachableInstrs(SuccBB,cursReachBrSet,Visited);
			}
		}else{
			if(TInst->getNumSuccessors()==1){
				BasicBlock * SuccBB = TInst->getSuccessor(0);
				if(!checkIfVisited(SuccBB,ReachBrSet,Visited)){
					unordered_set<BRExtendedInfo *> * cursReachBrSet = new unordered_set<BRExtendedInfo *>;
					cursReachBrSet->clear();
					for(auto it = ReachBrSet->begin(), eit = ReachBrSet->end(); it!=eit; ++it){
						cursReachBrSet->insert((*it));
					}

					calculateReachableInstrs(SuccBB,cursReachBrSet,Visited);
				}
			}else{
				assert(TInst->getNumSuccessors()==0);
			}
		}
	}
}

/*
 * Please, refer to the definition of parallel instructions in Listing-1 in the paper.
 * It operates on every backward branch. Check all other branches. If the branch of
 * interest is on the true-path of the a branch, then all instructions on the false-path
 * are added to parallel instructions (and like-wise if the branch is on the false-path).
 * the algorithm however excludes parallel paths that includes barriers as they would not
 * allow for inter-thread communication (i.e., given the barrier-divergence freedom assumption)
 * no barriers can be reached while under divergence.
 * Note that for each parallel instruction, the m_parallelInstructions stores a set of branch
 * instructions to which this parallel instructions refer to.
 */

void BackwardBranchInfo::calculateParallelInstrs()
{
	for (auto iter =getCondBranches()->begin(), eter = getCondBranches()->end(); iter != eter; ++iter){
		BRExtendedInfo * BrInfo = *iter;
		if(!BrInfo->isConditional()) continue;
		if(doesPathContainBB(m_Self,BrInfo->getTruePath())){
			unordered_set<BasicBlock*> * BarrierFreePath = BrInfo->get_barrier_free_path(BrInfo->getFalseBB(),BrInfo->getIPDOMBB());
			for(auto BBit=BarrierFreePath->begin(), BBet=BarrierFreePath->end(); BBit!=BBet; ++BBit){
				for(auto itI=(*BBit)->begin(),itIe=(*BBit)->end();itI!=itIe;++itI){
					if(m_parallelInstructions->find(itI)==m_parallelInstructions->end()){
						(*m_parallelInstructions)[itI] = new unordered_set<BRExtendedInfo *>;
					}
					m_parallelInstructions->at(itI)->insert(BrInfo);
				}
			}
		}
		if(doesPathContainBB(m_Self,BrInfo->getFalsePath())){
			unordered_set<BasicBlock*> * BarrierFreePath = BrInfo->get_barrier_free_path(BrInfo->getTrueBB(),BrInfo->getIPDOMBB());
			for(auto BBit=BarrierFreePath->begin(), BBet=BarrierFreePath->end(); BBit!=BBet; ++BBit){
				for(auto itI=(*BBit)->begin(),itIe=(*BBit)->end();itI!=itIe;++itI){
					if(m_parallelInstructions->find(itI)==m_parallelInstructions->end()){
						(*m_parallelInstructions)[itI] = new unordered_set<BRExtendedInfo *>;
					}
					m_parallelInstructions->at(itI)->insert(BrInfo);
				}
			}
		}
	}
}

unordered_set<BasicBlock*> * BRExtendedInfo::get_barrier_free_path(BasicBlock * src, BasicBlock * dst)
{
	unordered_set<BasicBlock*> * Path = new unordered_set<BasicBlock*>;
	unordered_set<BasicBlock*> * curPath = new unordered_set<BasicBlock*>;

	bool barrier_free = true;
	for(auto itI = src->begin(), etI = src->end(); itI != etI; ++itI){
		if(dyn_cast<CallInst>(itI)){
			CallInst * cI = dyn_cast<CallInst>(itI);
			if(!cI->isInlineAsm() && cI->getCalledFunction()){
				if(cI->isBarrierCall()){
					barrier_free = false;
					break;
				}
			}
		}else if(dyn_cast<BranchInst>(itI)){
			BranchInst * bI = dyn_cast<BranchInst>(itI);
			for(unsigned i=0; i<bI->getNumSuccessors();i++){
				/*
				 * This is specific to OpenMP and how current support deals with barriers.
				 * Barries at any locations are replaced by a branch instruction to single
				 * barrier at the end of the kernel.
				 */
				if(bI->getSuccessor(i)->getName().compare(".sync.and.next.state")==0){
					barrier_free = false;
					break;
				}
			}
			if(!barrier_free)
				break;
		}
	}


	if(barrier_free){
		if(src==dst){
			for(auto it = curPath->begin(), eit = curPath->end(); it!=eit; ++it){
				if(find(Path->begin(), Path->end(), (*it))==Path->end()){
					Path->insert((*it));
				}
			}
		}else{
			curPath->insert(src);
			unordered_set<BasicBlock*> * ncurPath = new unordered_set<BasicBlock*>;
			for(auto it = curPath->begin(), eit = curPath->end(); it!=eit; ++it){
				ncurPath->insert((*it));
			}

			TerminatorInst * TInst = src->getTerminator();
			for(unsigned i=0; i<TInst->getNumSuccessors(); i++){
				get_rec_barrier_free_path(TInst->getSuccessor(i), dst, ncurPath, Path);
			}
		}
	}

	return Path;

}

void BRExtendedInfo::get_rec_barrier_free_path(BasicBlock * nxt, BasicBlock * dst, unordered_set<BasicBlock*> * curPath, unordered_set<BasicBlock*> * Path)
{
	bool barrier_free = true;
	for(auto itI = nxt->begin(), etI = nxt->end(); itI != etI; ++itI){
		if(dyn_cast<CallInst>(itI)){
			CallInst * cI = dyn_cast<CallInst>(itI);
			if(!cI->isInlineAsm() && cI->getCalledFunction()){
				if(cI->isBarrierCall()){
					barrier_free = false;
					break;
				}
			}
		}else if(dyn_cast<BranchInst>(itI)){
			BranchInst * bI = dyn_cast<BranchInst>(itI);
			for(unsigned i=0; i<bI->getNumSuccessors();i++){
				/*
				 * This is specific to OpenMP and how current support deals with barriers.
				 * Barries at any locations are replaced by a branch instruction to single
				 * barrier at the end of the kernel.
				 */
				if(bI->getName().compare(".sync.and.next.state")){
					barrier_free = false;
					break;
				}
			}
			if(!barrier_free)
				break;
		}
	}


	if(barrier_free){
		if(nxt==dst){
			for(auto it = curPath->begin(), eit = curPath->end(); it!=eit; ++it){
				if(find(Path->begin(), Path->end(), (*it))==Path->end()){
					Path->insert((*it));
				}
			}
		}else{
			curPath->insert(nxt);
			unordered_set<BasicBlock*> * ncurPath = new unordered_set<BasicBlock*>;
			for(auto it = curPath->begin(), eit = curPath->end(); it!=eit; ++it){
				ncurPath->insert((*it));
			}

			TerminatorInst * TInst = nxt->getTerminator();
			for(unsigned i=0; i<TInst->getNumSuccessors(); i++){
				get_rec_barrier_free_path(TInst->getSuccessor(i), dst, curPath, Path);
			}
		}
	}

	return;
}





/*
 * Calculated ReqPDOM is guaranteed to post-dominate all redefining stores in reachable
 * or parallel paths along with all control flow paths that could lead to them. Check Algorithm-2.
 * */
void BackwardBranchInfo::calculateInitialReqPDOM()
{
	Instruction * ReqPDOMInst = nullptr;

	for(auto it = m_LoopExitConditions->begin(), et = m_LoopExitConditions->end(); it!=et; ++it){
		TerminatorInst *exitBranch = (*it);
		for(unsigned i =0; i<exitBranch->getNumSuccessors();i++){
			BasicBlock * Succ = exitBranch->getSuccessor(i);
			if(!ReqPDOMInst){
				ReqPDOMInst = Succ->begin();
			}else{
				ReqPDOMInst = getCommonPDOM(ReqPDOMInst,Succ->begin());
			}
		}
	}

	for(auto itI = m_PotentialRedefiningStoresSet->begin(), itIe = m_PotentialRedefiningStoresSet->end(); itI!=itIe; ++itI)
	{
		Instruction * curInst = (*itI);
		ReqPDOMInst = getCommonPDOM(ReqPDOMInst,curInst->getNextNode());

		if(m_parallelInstructions->find(curInst)!=m_parallelInstructions->end()){
			unordered_set<BRExtendedInfo *> * curParallelBrSet = m_parallelInstructions->at(curInst);
			for(auto itBr = curParallelBrSet->begin(), itBre = curParallelBrSet->end(); itBr!=itBre; ++itBr){
				BRExtendedInfo *BrInfo = (*itBr);
				BasicBlock *curBrRecBB = BrInfo->getStackBasedRec();
				Instruction*curBrRecInst = curBrRecBB->begin();
				ReqPDOMInst = getCommonPDOM(ReqPDOMInst,curBrRecInst);
			}
		}

		if(m_reachableInstructions->find(curInst)!=m_reachableInstructions->end()){
			unordered_set<BRExtendedInfo *> * curReachBrSet = m_reachableInstructions->at(curInst);
			for(auto itBr = curReachBrSet->begin(), itBre = curReachBrSet->end(); itBr!=itBre; ++itBr){
				BRExtendedInfo *BrInfo = (*itBr);
				BasicBlock *curBrRecBB = BrInfo->getStackBasedRec();
				Instruction*curBrRecInst = curBrRecBB->begin();
				ReqPDOMInst = getCommonPDOM(ReqPDOMInst,curBrRecInst);
			}
		}
	}

	setInitReqPDOM(ReqPDOMInst);
	setFinlReqPDOM(ReqPDOMInst);
}

/*
 * Relaxed ReqPDOM can be used with AWARE since aware relies on TimedOut reconvergence as a back-up mechanism. It is safe
 * to relax the ReqPDOM estimation avoiding overly conservative decisions. For example, Shrdwrites condiser only reachable
 * stores since it is communication with parallel paths is less common in pratice.
 */
void BackwardBranchInfo::calculateInitialReqPDOMRelaxed()
{
	Instruction * ReqPDOMInst = nullptr;

	for(auto it = m_LoopExitConditions->begin(), et = m_LoopExitConditions->end(); it!=et; ++it){
		TerminatorInst *exitBranch = (*it);
		for(unsigned i =0; i<exitBranch->getNumSuccessors();i++){
			BasicBlock * Succ = exitBranch->getSuccessor(i);
			if(!ReqPDOMInst){
				ReqPDOMInst = Succ->begin();
			}else{
				ReqPDOMInst = getCommonPDOM(ReqPDOMInst,Succ->begin());
			}
		}
	}

	for(auto itI = m_PotentialRedefiningStoresSetRelaxed->begin(), itIe = m_PotentialRedefiningStoresSetRelaxed->end(); itI!=itIe; ++itI)
	{
		Instruction * curInst = (*itI);
		ReqPDOMInst = getCommonPDOM(ReqPDOMInst,curInst->getNextNode());
		if(m_reachableInstructions->find(curInst)!=m_reachableInstructions->end()){
			unordered_set<BRExtendedInfo *> * curReachBrSet = m_reachableInstructions->at(curInst);
			for(auto itBr = curReachBrSet->begin(), itBre = curReachBrSet->end(); itBr!=itBre; ++itBr){
				BRExtendedInfo *BrInfo = (*itBr);
				BasicBlock *curBrRecBB = BrInfo->getStackBasedRec();
				Instruction*curBrRecInst = curBrRecBB->begin();
				ReqPDOMInst = getCommonPDOM(ReqPDOMInst,curBrRecInst);
			}
		}
	}

	setInitReqPDOMRelaxed(ReqPDOMInst);
}

void BackwardBranchInfo::print()
{
	if(m_createsLoop) errs()<<"Loop:\n";
	else return;

	errs() << "Retreating Edge: \n";
	m_BrInst->dump();

	errs()<<"Shared Load Inst Set				  : \n";
	for(auto itI = m_SharedLoadInstSet->begin(), itIe = m_SharedLoadInstSet->end(); itI!=itIe; ++itI)
	{
		errs()<<"										";
		(*itI)->dump();
	}

	errs()<<"Shared Store Inst Set				  : \n";
	for(auto itI = m_SharedStoreInstSet->begin(), itIe = m_SharedStoreInstSet->end(); itI!=itIe; ++itI)
	{
		errs()<<"										";
		(*itI)->dump();
	}

	errs()<<"Potential Redefining Stores		  : \n";
	for(auto itI = m_PotentialRedefiningStoresSet->begin(), itIe = m_PotentialRedefiningStoresSet->end(); itI!=itIe; ++itI)
	{
		errs()<<"										";
		(*itI)->dump();
	}

	errs()<<"Transformed: "<< needsTransformation << "\n";
	errs() << "Retreating Edge: \n";
	m_BrInst->dump();

	errs()<<"Initial Postdominator	        	  : \n";
	initReqPDOM->dump();

	errs()<<"Final Postdominator	        	  : \n";
	finlReqPDOM->dump();

}



void BRExtendedInfo::checkIfDominant()
{
	unordered_set<BasicBlock*> *PTtoIPDOM = new unordered_set<BasicBlock*>;
	unordered_set<BasicBlock*> *PFtoIPDOM = new unordered_set<BasicBlock*>;
	PTtoIPDOM->clear();
	PFtoIPDOM->clear();
	getPath(m_T, m_IPDOM, PTtoIPDOM);
	if(m_conditional){
		getPath(m_F, m_IPDOM, PFtoIPDOM);
	}
	bool dominatingTruePath = false;
	bool dominatingFalsePath = false;
	bool dominatedbyTruePath = false;
	bool dominatedbyFalsePath = false;

	dominatingTruePath = checkIfDominates(m_Self,PTtoIPDOM);
	dominatedbyTruePath = checkIfDominates(m_T,m_Self);
	if(m_conditional){
		dominatingFalsePath = checkIfDominates(m_Self,PFtoIPDOM);
		dominatedbyFalsePath = checkIfDominates(m_T,m_Self);
	}else{
		dominatingFalsePath = true;
		dominatedbyFalsePath = false;
	}

	m_Dominant = (dominatingTruePath and dominatingFalsePath) or (dominatedbyTruePath) or (dominatedbyFalsePath);
}

Instruction* FunctionHelperInfo::getCommonPDOM(Instruction *Inst1,Instruction *Inst2)
{
	BasicBlock * B1 = Inst1->getParent();
	BasicBlock * B2 = Inst2->getParent();
	Function *F = B1->getParent();
	assert(F == B2->getParent());
	BasicBlock * commPDOM = PDT->findNearestCommonDominator(B1,B2);
	if(B1==commPDOM && B2==commPDOM){
		//return the next node to the last one
		for(auto itI=commPDOM->begin(),itIe=commPDOM->end();itI!=itIe;++itI){
			Instruction *curInst = itI;
			if(curInst==Inst1){
				return Inst2;
			}
			if(curInst==Inst2){
				return Inst1;
			}
		}
	}else if(B1==commPDOM){
		return Inst1;
	}else if(B2==commPDOM){
		return Inst2;
	}

	return commPDOM->begin();
}


/*
 * Resolves conflicts between ReqPDOM (SafePDOM). SafePDOM of loop should
 * postdominates all SafePDOM of loops between the loop exists and its
 * SafePDOM, otherwise there is a conflict and SafePDOM will need to be
 * delayed further. Also, loops that had redefining writes within these bodies
 * that were not reachable nor parallel to the exit will be conservatively labled
 * as a potential cause for SIMT-induced deadlocks if their SafePDOM changes.
 *
 * do
 *	converged = true
 *	for each loop curL ∈ LSet do
 *		iSafePDOM(curL) = SafePDOM(curL)
 *		if curL causes potential SIMT-induced deadlocks then
 *			for each loop L, where BBs(L) ⊂ P Exits(curL)→SafePDom(curL) do
 *				SafePDom(curL) = IPDom(SafePDom(curL), SafePDom(L))
 *			end for
 *		else
 *			for each loop L, where BBs(L) ⊂ BBs(curL) do
 *				if SafePDOM(L) dominates SafePDOM(curL) then
 *					if curL Exits are dependent on shared variable then
 *						Label curL as a potential cause for SIMT-induced deadlocks
 *					end if
 *				end if
 *				SafePDom(curL) = IPDom(SafePDom(curL), SafePDom(L))
 *			end for
 *		end if
 *		if iSafePDOM(curL) = SafePDOM(curL) then
 *			converged = false
 *		end if
 *	end for
 * while converged = true
 */
void FunctionHelperInfo::recalculateReqPDOM()
{
	bool converged = true;
	do{
		converged = true;
		for (auto iter =m_RetreatingBranches->begin(), eter = m_RetreatingBranches->end(); iter != eter; ++iter){
			BackwardBranchInfo* BckwrdInfo = *iter;
			Instruction * iReqPDOMInst = BckwrdInfo->getFinlReqPDOM();
			Instruction * ReqPDOMInst = BckwrdInfo->getFinlReqPDOM();
			unordered_set<BasicBlock*> *PLtoReqPDOM = new unordered_set<BasicBlock*>;
			PLtoReqPDOM->clear();
			for(auto itIL = BckwrdInfo->getLoopExitNodes()->begin(), itILe = BckwrdInfo->getLoopExitNodes()->end(); itIL!=itILe; ++itIL){
				BasicBlock * Exit = *itIL;
				getPath(Exit, ReqPDOMInst->getParent(),PLtoReqPDOM);
			}
			if(BckwrdInfo->doesNeedTransformation()){
				for (auto curiter =m_RetreatingBranches->begin(), cureter = m_RetreatingBranches->end(); curiter != cureter; ++curiter){
					BackwardBranchInfo* curBckwrdInfo = *curiter;
					Instruction * curReqPDOMInst = curBckwrdInfo->getFinlReqPDOM();
					if(BckwrdInfo!=curBckwrdInfo){
						if(doesPathContainBB(curBckwrdInfo->getSelfBB(),PLtoReqPDOM) && (ReqPDOMInst->getParent()!=curBckwrdInfo->getSelfBB())){
							ReqPDOMInst=getCommonPDOM(ReqPDOMInst,curReqPDOMInst);
						}
					}
				}
			}else if(!BckwrdInfo->getPotentialRedefiningStoresWithinLoop()->empty()){
				for (auto curiter =m_RetreatingBranches->begin(), cureter = m_RetreatingBranches->end(); curiter != cureter; ++curiter){
					BackwardBranchInfo* curBckwrdInfo = *curiter;
					if(curBckwrdInfo->createsLoop()){
						Instruction * curReqPDOMInst = curBckwrdInfo->getFinlReqPDOM();
						if(BckwrdInfo!=curBckwrdInfo){
							if(doesPathContainBB(curBckwrdInfo->getSelfBB(),BckwrdInfo->getLoopBBs())  && (ReqPDOMInst->getParent()!=curBckwrdInfo->getSelfBB()) ){
								if((curBckwrdInfo->doesNeedTransformation()) && (curReqPDOMInst==ReqPDOMInst)){
									BckwrdInfo->setNeedsTransformation();
								}
								ReqPDOMInst=getCommonPDOM(ReqPDOMInst,curReqPDOMInst);
							}
						}
						if(ReqPDOMInst!=BckwrdInfo->getInitReqPDOM()){
							BckwrdInfo->setNeedsTransformation();
						}
					}
				}
			}
			BckwrdInfo->setFinlReqPDOM(ReqPDOMInst);
			if(iReqPDOMInst != ReqPDOMInst){
				converged = false;
			}

		}
	}while(!converged);

}








void FunctionHelperInfo::recalculateReqPDOMRelaxed()
{
	bool converged = true;
	do{
		converged = true;
		for (auto iter =m_CondBranches->begin(), eter = m_CondBranches->end(); iter != eter; ++iter){
			BRExtendedInfo* BrInfo = *iter;
			Instruction * iReqPDOMInst = BrInfo->getFinalIPDOMRelaxed();
			Instruction * ReqPDOMInst = BrInfo->getFinalIPDOMRelaxed();
			unordered_set<BasicBlock*> *PBtoReqPDOM = new unordered_set<BasicBlock*>;
			PBtoReqPDOM->clear();
			getPath(BrInfo->getSelfBB(), ReqPDOMInst->getParent(),PBtoReqPDOM);
			for (auto curiter =m_CondBranches->begin(), cureter = m_CondBranches->end(); curiter != cureter; ++curiter){
				BRExtendedInfo* curBrInfo = *curiter;
				Instruction * curReqPDOMInst = curBrInfo->getFinalIPDOMRelaxed();
				if(BrInfo!=curBrInfo){
					if(doesPathContainBB(curBrInfo->getSelfBB(),PBtoReqPDOM) && (ReqPDOMInst->getParent()!=curBrInfo->getSelfBB())){
						ReqPDOMInst=getCommonPDOM(ReqPDOMInst,curReqPDOMInst);
					}
				}
			}
			BrInfo->setFinalIPDOMRelaxed(ReqPDOMInst);
			if(iReqPDOMInst != ReqPDOMInst){
				converged = false;
			}

		}
	}while(!converged);

}










bool FunctionHelperInfo::isRetreatingEdge(BasicBlock * B1, BasicBlock * B2)
{
	EdgeType t = m_SpanningTree->get_edge(B1,B2)->get_edge_type();
	if(t==Retreating){
		return true;
	}
	return false;
}

bool FunctionHelperInfo::isBackwardEdge(BasicBlock * B1, BasicBlock * B2)
{
	RetreatingEdgeType t = m_SpanningTree->get_edge(B1,B2)->get_retreating_edge_type();
	if(t==Backward){
		return true;
	}
	return false;
}


bool FunctionHelperInfo::checkIfDominates(BasicBlock * B1, BasicBlock * B2)
{
	Function * F = B1->getParent();
	assert(B2->getParent()==F);
	bool self = (B1==B2);
	bool dominates = (DT->dominates(B1,B2));
	return dominates || self;
}

bool FunctionHelperInfo::checkIfDominates(BasicBlock * B1,unordered_set<BasicBlock*> *BBunordered_set)
{
	Function * F = B1->getParent();
	for (unordered_set<BasicBlock *>::iterator BBI = BBunordered_set->begin(), BBE = BBunordered_set->end(); BBI != BBE; ++BBI)
	{
		BasicBlock * B2 = *BBI;
		assert(B1->getParent()==F);
		if(!DT->dominates(B1,B2))
			return false;
	}
	return true;
}


void FunctionHelperInfo::calculateDominanceLevels()
{
	for(auto curBBit = m_Function->begin(), curBBet = m_Function->end(); curBBit!=curBBet; ++curBBit)
	{
		BasicBlock * curBB = curBBit;
		(*m_dominanceBBs)[curBB] = new unordered_set<BasicBlock *>;
		unsigned level = 0;
		for(auto BBit = m_Function->begin(), BBet = m_Function->end(); BBit!=BBet; ++BBit)
		{
			BasicBlock * BB = BBit;
			if(curBB!=BB){
				if(DT->dominates(BB->begin(),curBB->begin())){
					level++;
					m_dominanceBBs->at(curBB)->insert(BB);
				}
			}
		}
		(*m_dominanceLevels)[curBB] = level;
	}
}

/*
 * This is the starting point of computing Algorithm-1 (The detection algorithm) pre-requisites
 * as well as the Algorithm implementation itself to detect potential SIMT-induced deadlocks
 * it also calls recalculateReqPDOM which computes the delayed reconvergence points
 */
void FunctionHelperInfo::getConditionalBranchesInfo()
{
	/*
	 * This loop simply extracts the branch instructions within the function and pre-compute
	 * some of the relevant information for each branch instruction
	*/
	for (auto itBB = m_Function->begin(), etBB = m_Function->end(); itBB != etBB; ++itBB){
		for(auto itI = itBB->begin(), etI = itBB->end(); itI != etI; ++itI){
			if(dyn_cast<BranchInst>(itI)){
				BranchInst *I = dyn_cast<BranchInst>(itI);
				if(I->isConditional()){
					BRExtendedInfo * BRInfo = new BRExtendedInfo(I,true,this);
					BasicBlock * T_BB = dyn_cast<BasicBlock>(I->getOperand(2));
					BasicBlock * F_BB = dyn_cast<BasicBlock>(I->getOperand(1));
					BasicBlock * IPDOM_BB = PDT->findNearestCommonDominator(T_BB,F_BB);
					BRInfo->setTrueBB(T_BB);
					BRInfo->setFalseBB(F_BB);
					BRInfo->setIPDOMBB(IPDOM_BB);
					BRInfo->setSelfBB(I->getParent());
					BRInfo->checkIfDominant();
					BRInfo->setStackBasedRec(IPDOM_BB);
					BRInfo->setFinalIPDOMRelaxed(IPDOM_BB->begin());
					BRInfo->setinitIPDOMRelaxed(IPDOM_BB->begin());
					m_CondBranches->push_back(BRInfo);
				}else{
					BRExtendedInfo * BRInfo = new BRExtendedInfo(I,false,this);
					BasicBlock * T_BB = dyn_cast<BasicBlock>(I->getOperand(0));
					BasicBlock * IPDOM_BB = PDT->findNearestCommonDominator(T_BB,I->getParent());
					BRInfo->setTrueBB(T_BB);
					BRInfo->setFalseBB(nullptr);
					BRInfo->setIPDOMBB(IPDOM_BB);
					BRInfo->setSelfBB(I->getParent());
					BRInfo->checkIfDominant();
					BRInfo->setStackBasedRec(IPDOM_BB);
					BRInfo->setFinalIPDOMRelaxed(IPDOM_BB->begin());
					BRInfo->setinitIPDOMRelaxed(IPDOM_BB->begin());
					m_CondBranches->push_back(BRInfo);
				}
			}
		}
	}

	/*
	 * 1) Compute Pt->r and Pnt->r for each branch (which is used later to compute parallel and reachable
	 *    code regions yo the different loops.
	 * 2) Compute loop body for branches that has retreating edges
	*/
	for (auto iter =m_CondBranches->begin(), eter = m_CondBranches->end(); iter != eter; ++iter){
		BRExtendedInfo * BrInfo = *iter;
		BRExtendedInfo * BrInfo_earliestDominant = nullptr;
		/* The next code is to find the closed dominant conditional branch for the non-dominant branches,
		 * in this case, the reconvergence point of the non-dominant branch will be set to that of the closest dominant branch.
		 * This is specific to Nvidia reconvergence constraints, where only branches that dominate thri immediate postdominators
		 * will have a reconvergence point their. Otherwise, no reconvergence is set.
		 */

		if(!BrInfo->isDominant()){
			BrInfo->setNvidiaBasedRec(BrInfo->getIPDOMBB());
		}
	}
	for (auto iter =m_CondBranches->begin(), eter = m_CondBranches->end(); iter != eter; ++iter){
		BRExtendedInfo * BrInfo1 = *iter;
		BRExtendedInfo * BrDom = nullptr;
		if(!BrInfo1->isDominant()){
			for (auto iter1 =m_CondBranches->begin(), eter1 = m_CondBranches->end(); iter1 != eter1; ++iter1){
				BRExtendedInfo * BrInfo2 = *iter1;
				if(BrInfo2->isDominant()){
					if(DT->dominates(BrInfo2->getBrInst(),BrInfo1->getBrInst()) && PDT->dominates(BrInfo2->getIPDOMBB(),BrInfo1->getSelfBB())){
						if(!BrDom || DT->dominates(BrDom->getBrInst(),BrInfo2->getBrInst())){
							BrDom = BrInfo2;
						}
					}

				}
			}
			assert(BrDom);
			BrInfo1->setNvidiaBasedRec(BrDom->getIPDOMBB());
		}
	}

	for (auto iter =m_CondBranches->begin(), eter = m_CondBranches->end(); iter != eter; ++iter){
		BRExtendedInfo * BrInfo = *iter;

		BrInfo->calculateTruePath();
		if(BrInfo->isConditional()){
			BrInfo->calculateFalsePath();
		}

		/* Detecting Loop bodies */
		if(isRetreatingEdge(BrInfo->getSelfBB(),BrInfo->getTrueBB())){
			BackwardBranchInfo * BckwrdBr = new BackwardBranchInfo(BrInfo,this);
			BckwrdBr->setTakenAndNotTakenTargets(BrInfo->getTrueBB(),BrInfo->getFalseBB());
			BckwrdBr->calculateLoopBBs();
			m_RetreatingBranches->insert(BckwrdBr);
		}
		if(BrInfo->isConditional()){
			if(isRetreatingEdge(BrInfo->getSelfBB(),BrInfo->getFalseBB())){
				BackwardBranchInfo * BckwrdBr = new BackwardBranchInfo(BrInfo,this);
				BckwrdBr->setTakenAndNotTakenTargets(BrInfo->getFalseBB(),BrInfo->getTrueBB());
				BckwrdBr->calculateLoopBBs();
				m_RetreatingBranches->insert(BckwrdBr);
			}
		}
	}

	for(auto it=m_RetreatingBranches->begin(), et=m_RetreatingBranches->end(); it!=et; ++it)
	{
		BackwardBranchInfo * curBckwrdBr = (*it);
		for(auto it=m_RetreatingBranches->begin(), et=m_RetreatingBranches->end(); it!=et; ++it)
		{
			BackwardBranchInfo * BckwrdBr = (*it);
			if((curBckwrdBr!=BckwrdBr) && (BckwrdBr->getTakenBB()==curBckwrdBr->getTakenBB())){
				//merge loops
				for(auto lit=BckwrdBr->getLoopBBs()->begin(), let=BckwrdBr->getLoopBBs()->end(); lit!=let; ++lit)
				{
					curBckwrdBr->addLoopBB((*lit));
				}
			}
		}
	}

	/*
	 *  Calculate all pre-requisites and the initial ReqPDOM
	 */
	for(auto it=m_RetreatingBranches->begin(), et=m_RetreatingBranches->end(); it!=et; ++it)
	{
		BackwardBranchInfo * BckwrdBr = (*it);
		if(BckwrdBr->createsLoop()){
			/*  Calculate all pre-requisites*/
			BckwrdBr->calculateLoopExitNodes();
			BckwrdBr->calculateLoopInstrs();
			BckwrdBr->calculateReachableInstrs();
			BckwrdBr->calculateParallelInstrs();

			/* Backward slicing */
			BckwrdBr->calculateDependencyLog();

			/* ShrdReads */
			BckwrdBr->calculateSharedLoadInstSet();

			/* ShrdWrites */
			BckwrdBr->calculateSharedStoreInstSet();
			BckwrdBr->calculateSharedStoreInstSetWithinLoop();
			BckwrdBr->calculateSharedStoreInstSetRelaxed();

			/* RedWrites */
			BckwrdBr->calculateRedefiningSharedStoreInstSet();
			BckwrdBr->calculateRedefiningSharedStoreInstSetWithinLoop();
			BckwrdBr->calculateRedefiningSharedStoreInstSetRelaxed();

			/* ReqPDOM Calculation */
			BckwrdBr->calculateInitialReqPDOM();
			BckwrdBr->calculateInitialReqPDOMRelaxed();
			BckwrdBr->getBrInfo(BckwrdBr->getBrInst())->setinitIPDOMRelaxed(BckwrdBr->getInitReqPDOMRelaxed());
			BckwrdBr->getBrInfo(BckwrdBr->getBrInst())->setFinalIPDOMRelaxed(BckwrdBr->getInitReqPDOMRelaxed());
			BckwrdBr->initNeedsTransformation();
		}
	}

	/* Resolve Conflicts between ReqPDOMs of different loops */
	recalculateReqPDOM();
	recalculateReqPDOMRelaxed();

}


bool FunctionHelperInfo::checkIfLoadAliasesWithStore(Instruction *L, Instruction *S)
{
	Value * V1= nullptr;
	Value * V2 = nullptr;
	unsigned addressSpace1 = (unsigned)-1;
	unsigned addressSpace2 = (unsigned)-1;
	if(dyn_cast<LoadInst>(L)){
		LoadInst *LI=dyn_cast<LoadInst>(L);
		V1 = LI->getPointerOperand();
		addressSpace1 = LI->getPointerAddressSpace();
	}else if(dyn_cast<AtomicCmpXchgInst>(L)){
		AtomicCmpXchgInst *LI=dyn_cast<AtomicCmpXchgInst>(L);
		V1 = LI->getPointerOperand();
		addressSpace1 = LI->getPointerAddressSpace();
	}else if(dyn_cast<AtomicRMWInst>(L)){
		AtomicRMWInst *LI=dyn_cast<AtomicRMWInst>(L);
		V1 = LI->getPointerOperand();
		addressSpace1 = LI->getPointerAddressSpace();
	}else if(dyn_cast<CallInst>(L)){
		CallInst *cI = dyn_cast<CallInst>(L);
		if(cI->isMemCpy()){
			V1 = cI->getArgOperand(1);
			addressSpace1 = cI->getArgOperand(1)->getType()->getPointerAddressSpace();
		}else if(cI->isAtomicCall()){
			V1 = cI->getArgOperand(1);
			addressSpace1 = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
		}
	}

	if(dyn_cast<StoreInst>(S)){
		StoreInst *SI=dyn_cast<StoreInst>(S);
		V2 = SI->getPointerOperand();
		addressSpace2 = SI->getPointerAddressSpace();
	}else if(dyn_cast<AtomicCmpXchgInst>(S)){
		AtomicCmpXchgInst *SI=dyn_cast<AtomicCmpXchgInst>(S);
		V2 = SI->getPointerOperand();
		addressSpace2 = SI->getPointerAddressSpace();
	}else if(dyn_cast<AtomicRMWInst>(S)){
		AtomicRMWInst *SI=dyn_cast<AtomicRMWInst>(S);
		V2 = SI->getPointerOperand();
		addressSpace2 = SI->getPointerAddressSpace();
	}else if(dyn_cast<CallInst>(S)){
		CallInst *cI = dyn_cast<CallInst>(S);
		if(cI->isMemCpy()){
			V2 = cI->getArgOperand(0);
			addressSpace1 = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
		}else if(cI->isAtomicCall()){
			V1 = cI->getArgOperand(1);
			addressSpace1 = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
		}
	}


    if(addressSpace1!=addressSpace2) return false;
    AliasAnalysis::AliasResult ar=AA->alias(V1,V2);
    if(ar!=AliasAnalysis::NoAlias){
        return true;
    }else{
        return false;
    }

}

