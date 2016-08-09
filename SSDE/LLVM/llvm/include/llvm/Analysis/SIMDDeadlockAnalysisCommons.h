#ifndef LLVM_ANALYSIS_SIMDDEADLOACKS_H
#define LLVM_ANALYSIS_SIMDDEADLOACKS_H
#include "llvm/Pass.h"
//#include "llvm/PassManager.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Support/GenericDomTree.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/../../lib/Target/NVPTX/MCTargetDesc/NVPTXBaseInfo.h"
using namespace std;
#include <list>
#include <stack>
#include <string>
#include <unordered_set>




#define MAX_MAIN_FUNCTIONS 8
#define MAX_DOMINANCE_DESCENDANTS 20
namespace llvm {
	class BRExtendedInfo;
	class BackwardBranchInfo;
	class SpanningTree;
	class FunctionHelperInfo;
	struct Node;
	struct Edge;
	struct Graph;

	enum EdgeType { NAET = 0, Advancing = 1, Cross = 2, Retreating = 3 };
	enum AdvancingEdgeType { NAAET = 0, Tree = 1, Forward = 2 };
	enum RetreatingEdgeType { NARET = 0, Backward = 1, NotBackward = 2 };
	struct Node{
	public:
		Node(BasicBlock * iBB)
		{
			m_BB = iBB;
			m_parent = nullptr;
			m_ancestors = new unordered_set<Node *>;
			m_descendants = new unordered_set<Node *>;
			m_proper_descendants = new unordered_set<Node *>;
			m_outgoing_spt_edges = new unordered_set<Edge *>;
		}
		BasicBlock * getBB() {return m_BB;}
		Node * getParent() { return m_parent;}
		unordered_set<Node *> * get_proper_descendants() { return m_proper_descendants;}
		unordered_set<Node *> * get_descendants() { return m_descendants;}
		unordered_set<Node *> * get_ancestors() { return m_ancestors;}

		void setParent(Node * parent) { m_parent = parent; }
		void add_outgoing_spt_edge(Edge *edge) { m_outgoing_spt_edges->insert(edge); }
		void add_ancestor(Node * anscestor) { m_ancestors->insert(anscestor); }
		void add_descendant(Node * descendant) { m_descendants->insert(descendant); }
		void add_proper_descendant(Node * proper_descendant) { m_proper_descendants->insert(proper_descendant); }
		void dump()
		{
			errs() << "******************\n";
			errs() << "Node BB:";
			m_BB->dump();
			errs() << "Node parent: ";
			if(m_parent){
				m_parent->getBB()->dump();
			}else{
				errs()<<"no parent\n";
			}
			errs() << "proper descendants:";
			for(auto it=m_proper_descendants->begin(), et=m_proper_descendants->end(); it!=et; ++it){
				Node * n = *it;
				n->getBB()->dump();
			}
			errs() << "\n******************\n";
		}
	private:
		BasicBlock * m_BB;
		unordered_set<Edge *> * m_outgoing_spt_edges;
		Node * m_parent;
		unordered_set<Node *> * m_ancestors;
		unordered_set<Node *> * m_descendants;
		unordered_set<Node *> * m_proper_descendants;
	};
	struct Edge{
	public:
		Edge(Node *isrc, Node *itrg, BranchInst *ibrInst)
		{
			m_src = isrc;
			m_trg = itrg;
			m_brInst = ibrInst;
			m_edge_type = NAET;
			m_advancing_edge_type = NAAET;
			m_retreating_edge_type = NARET;
		}

		Node* getSrc() { return m_src;}
		Node* getTrg() { return m_trg;}
		void set_edge_type(EdgeType t) {m_edge_type = t;}
		void set_advancing_edge_type(AdvancingEdgeType t) {m_advancing_edge_type = t;}
		void set_retreating_edge_type(RetreatingEdgeType t) {m_retreating_edge_type = t;}
		void dump()
		{
			errs() << "******************\n";
			errs() << "Src Node BB:";
			m_src->getBB()->dump();
			errs() << "Trg Node BB: ";
			m_trg->getBB()->dump();
			errs() << "Edge Type:\n";
			if(m_edge_type==Advancing){
				errs() << "Advancing\n";
			}else if(m_edge_type==Cross){
				errs() << "Cross\n";
			}else if(m_edge_type==Retreating){
				errs() << "Retreating\n";
			}
			errs() << "******************\n";
		}
		EdgeType get_edge_type() {return m_edge_type;}
		AdvancingEdgeType get_advancing_edge_type() {return m_advancing_edge_type;}
		RetreatingEdgeType get_retreating_edge_type() {return m_retreating_edge_type;}
	private:
		Node * m_src;
		Node * m_trg;
		BranchInst * m_brInst;
		EdgeType m_edge_type;
		AdvancingEdgeType m_advancing_edge_type;
		RetreatingEdgeType m_retreating_edge_type;
	};
	struct Graph{
	public:
		Graph()
		{
			m_nodes = new map<BasicBlock *, Node *>;
			m_edges = new unordered_set<Edge *>;
		}
		void add_node(BasicBlock * BB,Node * node) { (*m_nodes)[BB]=node; }
		void add_edge(Edge * edge) { m_edges->insert(edge); }
		unordered_set<Edge *> * get_edges() {return m_edges;}
		map<BasicBlock *, Node *> * get_nodes() {return m_nodes;}
		void dump()
		{
			errs() << "Graph Nodes:\n";
			for(auto it=m_nodes->begin(), et=m_nodes->end(); it!=et; ++it){
				Node * curNode = it->second;
				curNode->dump();
			}

			errs() << "Graph Edges:\n";
			for(auto it=m_edges->begin(), et=m_edges->end(); it!=et; ++it){
				Edge * curEdge = *it;
				curEdge->dump();
			}
		}
		Edge* get_edge(BasicBlock *isrc, BasicBlock *itrg)
		{
			for(auto it=m_edges->begin(), et=m_edges->end(); it!=et; ++it){
				Edge * curEdge = *it;
				BasicBlock * src = curEdge->getSrc()->getBB();
				BasicBlock * trg = curEdge->getTrg()->getBB();
				if((src==isrc) && (trg==itrg)){
					return curEdge;
				}
			}
			return nullptr;
		}
		Node * get_node(BasicBlock * BB) { return m_nodes->at(BB); }
	private:
		map<BasicBlock *, Node *> * m_nodes;
		unordered_set<Edge *> * m_edges;
	};

	class SpanningTree {
	public:
    	SpanningTree(Function *iF, DominatorTree *iDT);
    	void STDFS(BasicBlock * curBB);
    	void STDFS(BasicBlock * curBB, unordered_set<BasicBlock *> * visitedNodes);
    	void calculate_st_ancestors();
    	void calculate_st_descendants();
    	void calculate_st_ancestors(Node *n);
    	void calculate_st_descendants(Node *n);
    	void calculate_st_descendants(Node *n, Node *curNode);
    	void classifyEdges();
    	void dump();
    	Edge * get_edge(BasicBlock *BB1, BasicBlock *BB2) {return m_CFG->get_edge(BB1,BB2);}
    	Node * get_node(BasicBlock *BB) {return m_CFG->get_node(BB);}

	private:
    	Graph *m_CFG;
    	Graph *m_SpanningTree;
    	Function * m_F;
    	DominatorTree * m_DT;
	};

	class FunctionHelperInfo
	{
		public:
		FunctionHelperInfo(Function *F,AliasAnalysis *iAA,PostDominatorTree * iPDT,DominatorTreeWrapperPass *iDTP,DominatorTree *iDT)
		{
			m_Function = F;
			PDT = iPDT;
			DTP = iDTP;
			DT  = iDT;
			AA  = iAA;
			m_CondBranches = new vector<BRExtendedInfo *>;
			m_RetreatingBranches = new unordered_set<BackwardBranchInfo*>;
			m_CondBranches->clear();
			m_RetreatingBranches->clear();
			m_SpanningTree = new SpanningTree(F,iDT);
	    	m_dominanceLevels = new map<BasicBlock*,unsigned>;
	    	m_dominanceBBs = new map<BasicBlock*,unordered_set<BasicBlock *>*>;
	    	calculateDominanceLevels();
		}
		FunctionHelperInfo(FunctionHelperInfo* FuncHelpInfo)
		{
			m_Function = FuncHelpInfo->getFunction();
			PDT = FuncHelpInfo->getPDT();
			DTP = FuncHelpInfo->getDTP();
			DT  = FuncHelpInfo->getDT();
			AA  = FuncHelpInfo->getAA();
			m_CondBranches = FuncHelpInfo->getCondBranches();
			m_RetreatingBranches = FuncHelpInfo->getRetreatingBranches();
			m_SpanningTree = FuncHelpInfo->getSpanningTree();
	    	m_dominanceLevels = FuncHelpInfo->getDominanceLevels();
	    	m_dominanceBBs = FuncHelpInfo->getDominanceBBs();
		}
		unsigned getDominanceLevel(BasicBlock *BB){ return (*m_dominanceLevels)[BB];}
		void calculateDominanceLevels();
		void getConditionalBranchesInfo();
		void recalculateReqPDOM();
		void recalculateReqPDOMRelaxed();
		void createSpanningTree();
		//Helper Functions
		bool isRetreatingEdge(BasicBlock *B1, BasicBlock *B2);
		bool isBackwardEdge(BasicBlock *B1, BasicBlock *B2);
		BRExtendedInfo *getBrInfo(BranchInst *BrInst);
		void getPath(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2);
		bool getPathBBs(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2);
		bool getPathBBsRecursive(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2, unordered_set<BasicBlock*> *checked);
		void getBackwardPath(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2);
		bool getBackwardPathBBs(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2);
		bool getBackwardPathBBsRecursive(BasicBlock *B1, BasicBlock* B2, unordered_set<BasicBlock*> *P1to2, unordered_set<BasicBlock*> *checked);
		bool doesPathContainBB(BasicBlock* B, unordered_set<BasicBlock*> *Path);
		bool checkIfDominates(BasicBlock * B1,unordered_set<BasicBlock*> *BBunordered_set);
		bool checkIfDominates(BasicBlock * B1, BasicBlock * B2);
		void trackOperandDependencies(Value* iOp, Value* Op, unordered_set<Value* > *DependencyLog, unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,unordered_set<Instruction*> * LoopInstructions,bool withinLoop);
		void extractDataDependencies(Value* iOp, Value* Op, unordered_set<Value* > *DependencyLog, unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,unordered_set<Instruction*> * LoopInstructions,bool withinLoop);
		void extractControlDependencies(Value* iOp, Value *Op, unordered_set<Value* > *DependencyLog,unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,unordered_set<Instruction*> * LoopInstructions,bool withinLoop);
		void extractPHIDependencies(Value* iOp, BasicBlock *OrgPHIBB, BasicBlock *PHIBB, Value *Op, unordered_set<Value* > *DependencyLog,unordered_set<Value* > *VisitedControlDependencies,unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<BasicBlock *> *VisitedPHIContributingNodes,unordered_set<Instruction*> * LoopInstructions,bool withinLoop);
		void extractStoreDependencies(Value* iOp, BasicBlock *PHIBB, Value *Op, unordered_set<Value* > *DependencyLog,unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,unordered_set<BasicBlock *> *VisitedPHIContributingNodes,unordered_set<Instruction*> * LoopInstructions,bool withinLoop);
		void extractDataDependenciesForAllocInst(Value* iOp, Value * Op, unordered_set<Value* > *DependencyLog,unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,unordered_set<Instruction*> * LoopInstructions,bool withinLoop);
		void extractDataDependenciesForCallInst(Value* iOp, Value *Op, unordered_set<Value* > *DependencyLog, unordered_set<Value *> *VisitedInstructionDependencies,unordered_set<Value* > *VisitedControlDependencies,unordered_set<Instruction*> * LoopInstructions,bool withinLoop);
		bool checkIfLoadAliasesWithStore(Instruction *L, Instruction *S);
		Instruction* getCommonPDOM(Instruction *Inst1,Instruction *Inst2);
		vector<BRExtendedInfo *>* getCondBranches() {return  m_CondBranches;}
		unordered_set<BackwardBranchInfo *>* getRetreatingBranches() {return  m_RetreatingBranches;}
		Function   * getFunction() {return m_Function;}
		PostDominatorTree * getPDT() {return PDT;}
		DominatorTreeWrapperPass *getDTP() {return DTP;}
		DominatorTree * getDT() {return DT;}
		AliasAnalysis * getAA() {return AA;}
		SpanningTree * getSpanningTree() {return m_SpanningTree;}
		map<BasicBlock*,unsigned> * getDominanceLevels() {return m_dominanceLevels;}
		map<BasicBlock*,unordered_set<BasicBlock *>*> * getDominanceBBs() {return m_dominanceBBs;}
		void print();

		protected:
		Function * m_Function;
		PostDominatorTree * PDT;
		DominatorTreeWrapperPass *DTP;
		DominatorTree * DT;
		AliasAnalysis * AA;
		vector<BRExtendedInfo *>* m_CondBranches;
		unordered_set<BackwardBranchInfo*>* m_RetreatingBranches;
		SpanningTree * m_SpanningTree;
    	map<BasicBlock*,unsigned> *m_dominanceLevels;
    	map<BasicBlock*,unordered_set<BasicBlock *>*> *m_dominanceBBs;
	};

	class BRExtendedInfo: public FunctionHelperInfo
	{
		public:

		BRExtendedInfo(BranchInst * BrInst, bool conditional, FunctionHelperInfo * FuncHelpInfo):FunctionHelperInfo(FuncHelpInfo)
		{
			m_BrInst = BrInst;
			m_conditional = conditional;
			m_PTtoR = new unordered_set<BasicBlock*>;
			m_PFtoR = new unordered_set<BasicBlock*>;
			m_PTtoR->clear();
			m_PFtoR->clear();
		}
		BRExtendedInfo(BRExtendedInfo * BrInstInfo, FunctionHelperInfo * FuncHelpInfo):FunctionHelperInfo(FuncHelpInfo)
		{
			m_BrInst = BrInstInfo->getBrInst();
			m_conditional = BrInstInfo->isConditional();
			m_StackBasedRec = BrInstInfo->getStackBasedRec();
			m_NvidiaBasedRec = BrInstInfo->getNvidiaBasedRec();
			m_IPDOM = BrInstInfo->getIPDOMBB();
			m_T = BrInstInfo->getTrueBB();
			m_F = BrInstInfo->getFalseBB();
			m_Self = BrInstInfo->getSelfBB();
			m_Dominant = BrInstInfo->isDominant();
			m_PTtoR = BrInstInfo->getTruePath();
			m_PFtoR = BrInstInfo->getFalsePath();
		}

		unordered_set<BasicBlock*> * get_barrier_free_path(BasicBlock * src, BasicBlock * dst);
		void get_rec_barrier_free_path(BasicBlock * nxt, BasicBlock * dst, unordered_set<BasicBlock*> * curPath, unordered_set<BasicBlock*> * Path);
		void setTrueBB(BasicBlock * BB) {m_T = BB;}
		void setFalseBB(BasicBlock * BB) {m_F = BB;}
		void setSelfBB(BasicBlock * BB) {m_Self = BB;}
		void setIPDOMBB(BasicBlock * BB) {m_IPDOM = BB;}
		void setStackBasedRec(BasicBlock * BB) {m_StackBasedRec = BB;}
		void setNvidiaBasedRec(BasicBlock * BB) {m_NvidiaBasedRec = BB;}
		BranchInst   * getBrInst() {return m_BrInst;}
		BasicBlock * getTrueBB() {return m_T;}
		BasicBlock * getFalseBB() {return m_F;}
		BasicBlock * getSelfBB() {return m_Self;}
		BasicBlock * getIPDOMBB() {return m_IPDOM ;}
		void setFinalIPDOMRelaxed(Instruction * ReqPDOMInst) {m_finReqPDOMInstRelaxed=ReqPDOMInst;}
		void setinitIPDOMRelaxed(Instruction * ReqPDOMInst) {m_iReqPDOMInstRelaxed=ReqPDOMInst;}
		Instruction * getFinalIPDOMRelaxed() {return m_finReqPDOMInstRelaxed;}
		Instruction * getInitIPDOMRelaxed() {return m_iReqPDOMInstRelaxed;}

		BasicBlock * getStackBasedRec() {return m_StackBasedRec;}
		BasicBlock * getNvidiaBasedRec() {return m_NvidiaBasedRec;}
		unordered_set<BasicBlock*> * getTruePath(){return m_PTtoR;}
		unordered_set<BasicBlock*> * getFalsePath(){return m_PFtoR;}
		void checkIfDominant();
		bool isDominant() {return m_Dominant;}
		void calculateTruePath();
		void calculateFalsePath();
		bool isConditional() {return m_conditional;}
		void print();
		protected:
			BranchInst * m_BrInst;
			BasicBlock * m_StackBasedRec;
			BasicBlock * m_NvidiaBasedRec;
			BasicBlock * m_IPDOM;
			BasicBlock * m_T;
			BasicBlock * m_F;
			BasicBlock * m_Self;
			unordered_set<BasicBlock*> * m_PTtoR;
			unordered_set<BasicBlock*> * m_PFtoR;
			Instruction * m_iReqPDOMInstRelaxed;
			Instruction * m_finReqPDOMInstRelaxed;
			bool m_Dominant;
			bool m_conditional;
	};

	class BackwardBranchInfo: public BRExtendedInfo
	{
		public:
		BackwardBranchInfo(BRExtendedInfo * BrInstInfo,FunctionHelperInfo * FuncHelpInfo):BRExtendedInfo(BrInstInfo,FuncHelpInfo)
		{
			m_createsLoop = false;
			m_DependencyLog = new unordered_set<Value* >;
			m_DependencyLogWithinLoop = new unordered_set<Value* >;
			m_SharedLoadInstSet = new  unordered_set<Instruction*>;
			m_SharedStoreInstSet = new  unordered_set<Instruction*>;
			m_SharedStoreInstSetWithinLoop = new  unordered_set<Instruction*>;
			m_SharedStoreInstSetRelaxed = new  unordered_set<Instruction*>;
			m_PotentialRedefiningStoresSet = new  unordered_set<Instruction*>;
			m_PotentialRedefiningStoresSetWithinLoop = new  unordered_set<Instruction*>;
			m_PotentialRedefiningStoresSetRelaxed = new  unordered_set<Instruction*>;
			m_LoopInstructions = new unordered_set<Instruction*>;
			m_reachableInstructions = new map<Instruction*, unordered_set<BRExtendedInfo *> */*ReachBrSet*/>;
			m_parallelInstructions = new map<Instruction*, unordered_set<BRExtendedInfo *> */*ParalBrSet*/>;
			m_PotentialLoadStorePairs = new vector<pair<Instruction *, Instruction *> >;
			m_PotentialLoadStorePairsWithinLoop = new vector<pair<Instruction *, Instruction *> >;
			m_PotentialLoadStorePairsRelaxed = new vector<pair<Instruction *, Instruction *> >;
			m_LoopBBs = new unordered_set<BasicBlock *>;
			m_LoopExitNodes = new unordered_set<BasicBlock *>;
			m_LoopExitConditions = new unordered_set<TerminatorInst *>;
			needsTransformation = false;
		}
		void calculateLoopInstrs();
		void calculateReachableInstrs();
		void calculateReachableInstrs(BasicBlock *SuccBB,unordered_set<BRExtendedInfo *> * ReachBrSet,map<BasicBlock *,unordered_set<BRExtendedInfo *> *> * Visited);
		bool checkIfVisited(BasicBlock *SuccBB,unordered_set<BRExtendedInfo *> * ReachBrSet,map<BasicBlock *,unordered_set<BRExtendedInfo *> *> * Visited);
		void calculateParallelInstrs();
		void calculateDependencyLog();
		void calculateSharedLoadInstSet();
		void calculateSharedStoreInstSet();
		void calculateSharedStoreInstSetWithinLoop();
		void calculateSharedStoreInstSetRelaxed();
		void calculateRedefiningSharedStoreInstSet();
		void calculateRedefiningSharedStoreInstSetWithinLoop();
		void calculateRedefiningSharedStoreInstSetRelaxed();
		void calculateInitialReqPDOM();
		void calculateInitialReqPDOMRelaxed();
		void calculateLoopBBs();
		void calculateLoopExitNodes();
		void initNeedsTransformation() {needsTransformation=isPotentiallyRedefined();}
		void setNeedsTransformation() {needsTransformation=true;}
		bool doesNeedTransformation() {return needsTransformation;}
		void setTakenAndNotTakenTargets(BasicBlock * B1,BasicBlock * B2)
		{
			m_TakenTarget=B1;
			if(m_conditional){
				m_NotTakenTarget=B2;
			}else{
				m_NotTakenTarget=nullptr;
			}
		}
		BasicBlock * getTakenBB(){return m_TakenTarget;}
		BasicBlock * getNotTakenBB(){return m_NotTakenTarget;}
		unordered_set<BasicBlock*> * getLoopExitNodes() {return m_LoopExitNodes;}
		unordered_set<BasicBlock*> * getLoopBBs() {return m_LoopBBs;}
		void addLoopBB(BasicBlock *BB)
		{
			if(find(m_LoopBBs->begin(),m_LoopBBs->end(),BB)==m_LoopBBs->end()){
				m_LoopBBs->insert(BB);
			}
			m_createsLoop = true;
		}

		bool createsLoop() {return m_createsLoop;}
		bool isdependenton(Value * Op);
		bool isdependentonWithinLoop(Value * Op);
		Instruction * getInitReqPDOM() {return initReqPDOM;}
		Instruction * getFinlReqPDOM() {return finlReqPDOM;}
		void setFinlReqPDOM(Instruction *I) {finlReqPDOM=I;}
		void setInitReqPDOM(Instruction *I) {initReqPDOM=I;}
		void setInitReqPDOMRelaxed(Instruction *I) {initReqPDOMRelaxed=I;}
		Instruction * getInitReqPDOMRelaxed() {return initReqPDOMRelaxed;}

		unordered_set<Instruction*> * getPotentialRedefiningStores() {return m_PotentialRedefiningStoresSet;}
		unordered_set<Instruction*> * getPotentialRedefiningStoresWithinLoop() {return m_PotentialRedefiningStoresSetWithinLoop;}
		unordered_set<Instruction*> * getPotentialRedefiningStoresRelaxed() {return m_PotentialRedefiningStoresSetRelaxed;}
		bool isDependantOnSharedMemory()
		{
			for(auto it=m_DependencyLog->begin(),et=m_DependencyLog->end();it!=et;++it){
				if(dyn_cast<LoadInst>((*it)) || dyn_cast<AtomicRMWInst>((*it)) || dyn_cast<AtomicCmpXchgInst>((*it)))
				{
					unsigned addrspace = dyn_cast<LoadInst>((*it))          ? dyn_cast<LoadInst>((*it))->getPointerAddressSpace():
								dyn_cast<AtomicRMWInst>((*it))     ? dyn_cast<AtomicRMWInst>((*it))->getPointerAddressSpace():
								dyn_cast<AtomicCmpXchgInst>((*it)) ? dyn_cast<AtomicCmpXchgInst>((*it))->getPointerAddressSpace(): (unsigned)-1;
					if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
						return true;
					}
				}

				if(dyn_cast<CallInst>((*it))){
					CallInst * cI = dyn_cast<CallInst>((*it));
					if(!cI->isInlineAsm()){
						if(cI->isMemCpy()){
							unsigned addrspace = cI->getArgOperand(1)->getType()->getPointerAddressSpace();
							if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
								return true;
							}
						}else if(cI->isAtomicCall()){
							//e.g., llvm.nvvm.load.inc
							unsigned addrspace = cI->getArgOperand(0)->getType()->getPointerAddressSpace();
							if((addrspace==llvm::ADDRESS_SPACE_SHARED) || (addrspace==llvm::ADDRESS_SPACE_GLOBAL)){
								return true;
							}
						}
					}
				}
			}
			return false;
		}
		bool isDependantOnSharedMemoryWithinTheLoop() {return !m_SharedLoadInstSet->empty();};
		bool isPotentiallyRedefined() { return 	!m_PotentialRedefiningStoresSet->empty();}
		bool isPotentiallyRedefinedRelaxed() { return 	!m_PotentialRedefiningStoresSetRelaxed->empty();}

		void print();
		private:
		BasicBlock * m_TakenTarget;
		BasicBlock * m_NotTakenTarget;
		bool m_createsLoop;
		//The ReachUnder(n) set in the "Identifying Loops Using DJ Graphs"
		unordered_set<BasicBlock *> * m_LoopBBs;
		unordered_set<TerminatorInst *> * m_LoopExitConditions;
		unordered_set<BasicBlock *> * m_LoopExitNodes;
		unordered_set<Instruction*> * m_LoopInstructions;
		map<Instruction*, unordered_set<BRExtendedInfo *> */*ReachBrSet*/> * m_reachableInstructions;
		map<Instruction*, unordered_set<BRExtendedInfo *> */*ParalBrSet*/> * m_parallelInstructions;
		unordered_set<Value* > *m_DependencyLog;
		unordered_set<Value* > *m_DependencyLogWithinLoop;
		unordered_set<Instruction*> *m_SharedLoadInstSet;
		unordered_set<Instruction*> *m_SharedStoreInstSet;
		unordered_set<Instruction*> *m_SharedStoreInstSetWithinLoop;
		unordered_set<Instruction*> *m_SharedStoreInstSetRelaxed;
		unordered_set<Instruction*> *m_PotentialRedefiningStoresSet;
		unordered_set<Instruction*> *m_PotentialRedefiningStoresSetWithinLoop;
		unordered_set<Instruction*> *m_PotentialRedefiningStoresSetRelaxed;
		vector<pair<Instruction *, Instruction *> > *m_PotentialLoadStorePairs;
		vector<pair<Instruction *, Instruction *> > *m_PotentialLoadStorePairsWithinLoop;
		vector<pair<Instruction *, Instruction *> > *m_PotentialLoadStorePairsRelaxed;
		Instruction * initReqPDOM;
		Instruction * finlReqPDOM;
		Instruction * initReqPDOMRelaxed;
		bool needsTransformation;
	};

}  // end of anonymous namespace


#endif

