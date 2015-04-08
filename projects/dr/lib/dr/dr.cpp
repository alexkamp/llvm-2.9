/*
 * File: dr.c
 *
 * Description:
 *  This is a dr source file for a library.  It helps to demonstrate
 *  how to setup a project that uses the LLVM build system, header files,
 *  and libraries.
 */

#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <sstream>

using namespace llvm;

namespace {
  void computeReachableCallGraphNodes(const CallGraphNode *N,
                                      DenseSet<const CallGraphNode *> &Nodes) {
    if (Nodes.count(N) || !N)
      return;
    Nodes.insert(N);

    Function *F = N->getFunction();
    if (F && !F->isDeclaration()) {
      for (CallGraphNode::const_iterator CGI = N->begin(), CGE = N->end();
           CGI != CGE; ++CGI) {
        const CallGraphNode::CallRecord Record = *CGI;
        computeReachableCallGraphNodes(Record.second, Nodes);
      }
    }
  }

  static cl::opt<std::string> EntryPoints(
      "entries",
      cl::desc("The names of entry functions as a space separated list."),
      cl::value_desc("entries"), cl::init("main"));

  struct DumpReachables : public ModulePass {
    static char ID;
    DumpReachables() : ModulePass(ID) {};

    virtual void getAnalysisUsage(AnalysisUsage &Info) const {
      Info.setPreservesAll();
      Info.addRequired<CallGraph>();
    }

    virtual bool runOnModule(Module &M) {
      errs() << "Traversing call graph...\n";
      CallGraph &CG = getAnalysis<CallGraph>();

      DenseSet<const CallGraphNode*> Reachables;

      std::istringstream In(EntryPoints);
      std::string tmp;
      while (In >> tmp) {
        errs() << " -> entry: " << tmp << ": ";
        Function *F = M.getFunction(tmp);
        if (F) {
          computeReachableCallGraphNodes(CG[F], Reachables);
          errs() << "OK\n";
        } else {
          errs() << "NOT FOUND!\n";
        }
      }

      DenseSet<Function*> ReachableFuns;
      for (DenseSet<const CallGraphNode *>::iterator I = Reachables.begin(),
                                                     E = Reachables.end();
           I != E; ++I)
        ReachableFuns.insert((*I)->getFunction());

      errs() << "\nFound " << ReachableFuns.size() << " reachable functions:\n";
      for (DenseSet<Function*>::iterator I = ReachableFuns.begin(), E=ReachableFuns.end(); I != E; ++I) {
        Function *F = *I;

        errs() << " - ";
        if (F)
          outs() << (*I)->getName() << "\n";
        else
          outs() << "<null>\n";
      }

      return false;
    }
  };

  char DumpReachables::ID = 0;

  static RegisterPass<DumpReachables> X("dump-reachables",
                                        "Dump functions definitely "
                                        "reachable via calls from the given "
                                        "entry points.");
}
