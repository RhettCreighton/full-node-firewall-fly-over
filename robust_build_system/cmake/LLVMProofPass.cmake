# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# LLVMProofPass.cmake - LLVM compiler pass for compile-time verification

include_guard(GLOBAL)

# Find LLVM
find_package(LLVM QUIET)

if(LLVM_FOUND)
    message(STATUS "✓ LLVM ${LLVM_VERSION} found - enabling LLVM proof passes")
    
    # LLVM configuration
    include_directories(${LLVM_INCLUDE_DIRS})
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
    add_definitions(${LLVM_DEFINITIONS_LIST})
    
    # Create LLVM pass for proof verification
    set(LLVM_PASS_SOURCE "${CMAKE_BINARY_DIR}/FirefoxProofPass.cpp")
    
    file(WRITE ${LLVM_PASS_SOURCE} "
#include \"llvm/Pass.h\"
#include \"llvm/IR/Function.h\"
#include \"llvm/IR/Instructions.h\"
#include \"llvm/IR/Module.h\"
#include \"llvm/Support/raw_ostream.h\"
#include \"llvm/Transforms/IPO/PassManagerBuilder.h\"
#include \"llvm/IR/LegacyPassManager.h\"

using namespace llvm;

namespace {
  struct FirefoxProofPass : public FunctionPass {
    static char ID;
    bool RequiredFunctionFound = false;
    bool RequiredFunctionCalled = false;
    std::string RequiredFunction;
    
    FirefoxProofPass() : FunctionPass(ID) {
      const char* reqFunc = getenv(\"PROOF_REQUIRED_FUNCTION\");
      RequiredFunction = reqFunc ? reqFunc : \"firefox_draw_pixel\";
    }
    
    bool runOnFunction(Function &F) override {
      // Check if this is our required function
      if (F.getName() == RequiredFunction) {
        RequiredFunctionFound = true;
      }
      
      // Check all call sites
      for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
          if (CallInst *CI = dyn_cast<CallInst>(&I)) {
            Function *Callee = CI->getCalledFunction();
            if (Callee && Callee->getName() == RequiredFunction) {
              RequiredFunctionCalled = true;
              errs() << \"PROOF_PASS: Found call to \" << RequiredFunction 
                     << \" in \" << F.getName() << \"\\n\";
            }
          }
        }
      }
      
      return false;  // We don't modify the function
    }
    
    bool doFinalization(Module &M) override {
      if (!RequiredFunctionFound) {
        errs() << \"PROOF_FAIL: Required function '\" << RequiredFunction 
               << \"' not found in module\\n\";
        abort();
      }
      
      if (!RequiredFunctionCalled) {
        errs() << \"PROOF_FAIL: Required function '\" << RequiredFunction 
               << \"' exists but is never called\\n\";
        abort();
      }
      
      errs() << \"PROOF_SUCCESS: All requirements satisfied\\n\";
      return false;
    }
  };
}

char FirefoxProofPass::ID = 0;

// Register pass
static RegisterPass<FirefoxProofPass> X(
  \"firefox-proof\", 
  \"Firefox Compile-Time Proof Pass\",
  false,  // Only looks at CFG
  false   // Analysis pass
);

// Automatically enable with optimization
static void registerFirefoxProofPass(const PassManagerBuilder &,
                                    legacy::PassManagerBase &PM) {
  PM.add(new FirefoxProofPass());
}

static RegisterStandardPasses RegisterMyPass(
  PassManagerBuilder::EP_OptimizerLast,
  registerFirefoxProofPass
);
")
    
    # Compile LLVM pass
    add_library(FirefoxProofPass MODULE ${LLVM_PASS_SOURCE})
    target_compile_features(FirefoxProofPass PRIVATE cxx_std_11)
    set_target_properties(FirefoxProofPass PROPERTIES
        COMPILE_FLAGS \"-fno-rtti\"
        POSITION_INDEPENDENT_CODE ON
    )
    
    # Function to use LLVM pass verification
    function(add_llvm_proof_verification TARGET)
        # Get pass location
        get_target_property(PASS_LOCATION FirefoxProofPass LIBRARY_OUTPUT_DIRECTORY)
        
        # Add flags to load our pass
        target_compile_options(${TARGET} PRIVATE
            -Xclang -load
            -Xclang $<TARGET_FILE:FirefoxProofPass>
        )
        
        # Set environment for pass configuration
        set_property(TARGET ${TARGET} PROPERTY 
            RULE_LAUNCH_COMPILE 
            "PROOF_REQUIRED_FUNCTION=${ARGN} "
        )
        
        message(STATUS "✓ LLVM proof pass enabled for ${TARGET}")
    endfunction()
    
else()
    message(STATUS "⚠ LLVM not found - using fallback proof methods")
    
    function(add_llvm_proof_verification TARGET)
        # Fallback to regular verification
        add_optimized_compile_time_proof(${TARGET} ${ARGN})
    endfunction()
endif()

# Clang plugin for compile-time verification
function(create_clang_plugin_proof)
    set(PLUGIN_SOURCE "${CMAKE_BINARY_DIR}/ProofPlugin.cpp")
    
    file(WRITE ${PLUGIN_SOURCE} "
#include \"clang/Frontend/FrontendPluginRegistry.h\"
#include \"clang/AST/AST.h\"
#include \"clang/AST/ASTConsumer.h\"
#include \"clang/AST/RecursiveASTVisitor.h\"
#include \"clang/Frontend/CompilerInstance.h\"
#include \"llvm/Support/raw_ostream.h\"

using namespace clang;

namespace {

class ProofVisitor : public RecursiveASTVisitor<ProofVisitor> {
  ASTContext *Context;
  std::string RequiredFunction;
  bool FunctionFound = false;
  bool FunctionCalled = false;
  
public:
  explicit ProofVisitor(ASTContext *Context, const std::string &Func)
    : Context(Context), RequiredFunction(Func) {}
  
  bool VisitFunctionDecl(FunctionDecl *F) {
    if (F->getNameAsString() == RequiredFunction) {
      FunctionFound = true;
    }
    return true;
  }
  
  bool VisitCallExpr(CallExpr *Call) {
    if (FunctionDecl *F = Call->getDirectCallee()) {
      if (F->getNameAsString() == RequiredFunction) {
        FunctionCalled = true;
        llvm::errs() << \"PROOF: Found call to \" << RequiredFunction << \"\\n\";
      }
    }
    return true;
  }
  
  void verify() {
    if (!FunctionFound) {
      llvm::errs() << \"ERROR: Required function not found: \" 
                   << RequiredFunction << \"\\n\";
      exit(1);
    }
    if (!FunctionCalled) {
      llvm::errs() << \"ERROR: Required function never called: \" 
                   << RequiredFunction << \"\\n\";
      exit(1);
    }
    llvm::errs() << \"SUCCESS: Proof verification passed\\n\";
  }
};

class ProofConsumer : public ASTConsumer {
  ProofVisitor Visitor;
  
public:
  explicit ProofConsumer(ASTContext *Context, const std::string &Func)
    : Visitor(Context, Func) {}
  
  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    Visitor.verify();
  }
};

class ProofAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    std::string Func = \"firefox_draw_pixel\";  // Default
    if (!Args.empty()) {
      Func = Args[0];
    }
    return std::make_unique<ProofConsumer>(&CI.getASTContext(), Func);
  }
  
  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    Args = args;
    return true;
  }
  
private:
  std::vector<std::string> Args;
};

}

static FrontendPluginRegistry::Add<ProofAction>
X(\"proof-plugin\", \"Compile-time proof verification plugin\");
")
    
    # Build plugin if using Clang
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        add_library(ProofPlugin MODULE ${PLUGIN_SOURCE})
        target_compile_features(ProofPlugin PRIVATE cxx_std_14)
        
        # Function to use plugin
        function(add_clang_plugin_proof TARGET FUNCTION)
            target_compile_options(${TARGET} PRIVATE
                -Xclang -load
                -Xclang $<TARGET_FILE:ProofPlugin>
                -Xclang -plugin
                -Xclang proof-plugin
                -Xclang -plugin-arg-proof-plugin
                -Xclang ${FUNCTION}
            )
        endfunction()
    endif()
endfunction()