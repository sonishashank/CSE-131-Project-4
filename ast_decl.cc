/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"        
#include "ast.h"

llvm::Value* VarDecl::Emit(){
    llvm::Module *mod  = irgen->GetOrCreateModule("mod.bc");
    llvm::Type *t = irgen->GetType(this->GetType());

    if (symtable->globalScope == true){
        //TODO: check if vardecl is constant
        llvm::GlobalVariable *var = new llvm::GlobalVariable(*irgen->GetOrCreateModule("mod.bc"), t, false, llvm::GlobalValue::ExternalLinkage, llvm::Constant::getNullValue(t), this->GetIdentifier()->GetName());
    }
    
    else{
        //local variable
        //TODO: check if vardecl is constant
        llvm::AllocaInst(t, llvm::Constant::getNullValue(t), this->GetIdentifier()->GetName(), irgen->GetBasicBlock());
        
    }
    
    return NULL;
}
         
llvm::Value* FnDecl::Emit(){
    symtable->globalScope = false;
    llvm::Module *mod = irgen->GetOrCreateModule("mod.bc");
    llvm::Type *t = irgen->GetType(this->GetType());
    char* name = this->GetIdentifier()->GetName();

    std::vector<llvm::Type*> argTypes;
    List<VarDecl*> *args = this->GetFormals();
    for(int x = 0; x < args->NumElements(); x++){
        VarDecl* d = args->Nth(x);
        d->Emit();
        argTypes.push_back(irgen->GetType(d->GetType()));
    }
    llvm::ArrayRef<llvm::Type*> argArray(argTypes);

    Type *returnType = this->GetType();

    //TODO: third argument is bool isVarArg. What does that mean?
    llvm::FunctionType *funcTy = llvm::FunctionType::get(irgen->GetType(returnType), argArray, false);
    llvm::Function *f = llvm::cast<llvm::Function>(mod->getOrInsertFunction(name, funcTy));

    irgen->SetFunction(f);


    //TODO: loop through f to get the arg and set the name of the arg
    //llvm::Argument *arg = f->arg_begin();
    llvm::Function::arg_iterator arg = f->arg_begin();
    int x = 0;
    for(; arg != f->arg_end(); arg++){
        VarDecl* d = args->Nth(x);
        x++;
        arg->setName(d-> GetIdentifier() -> GetName());
    }
    
    //Creating and inserting a basic block into the function
    llvm::LLVMContext *context = irgen->GetContext();
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "entry", f);
    irgen->SetBasicBlock(bb);
    llvm::Value* returnExpr = this->body->Emit();
    return f;
}

Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

VarDecl::VarDecl(Identifier *n, Type *t, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    typeq = NULL;
}

VarDecl::VarDecl(Identifier *n, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && tq != NULL);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    type = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL && tq != NULL);
    (type=t)->SetParent(this);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   if (typeq) typeq->Print(indentLevel+1);
   if (type) type->Print(indentLevel+1);
   if (id) id->Print(indentLevel+1);
   if (assignTo) assignTo->Print(indentLevel+1, "(initializer) ");
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
    returnTypeq = NULL;
}

FnDecl::FnDecl(Identifier *n, Type *r, TypeQualifier *rq, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r != NULL && rq != NULL&& d != NULL);
    (returnType=r)->SetParent(this);
    (returnTypeq=rq)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
    if (returnType) returnType->Print(indentLevel+1, "(return type) ");
    if (id) id->Print(indentLevel+1);
    if (formals) formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}
