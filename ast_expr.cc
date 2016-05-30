/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"


/********** Arithmetic Expr Emit **********/
llvm::Value* ArithmeticExpr::Emit(){
  Operator *op = this->op;

  //pre increment
  if( this->left == NULL && this->right != NULL){
      //getting the value at the pointer
      llvm::Value *oldVal = this->right->Emit();

      //getting the pointer of the value
      llvm::LoadInst *l = llvm::cast<llvm::LoadInst>(oldVal);
      llvm::Value *location = l->getPointerOperand();

      //getting the basic block
      llvm::BasicBlock *bb = irgen->GetBasicBlock();
      //int case
      if( oldVal->getType() == irgen->GetIntType() ){
        llvm::Type* type = irgen->GetIntType();
        llvm::Constant* one = llvm::ConstantInt::get(type,1);
        llvm::Constant* negOne = llvm::ConstantInt::get(type,1, true);
        //int val = dynamic_cast<llvm::ConstantInt*>(oldVal)->getSExtValue();
        if( op->IsOp("++") ){
          //adding one
          llvm::Value *inc = llvm::BinaryOperator::CreateAdd(oldVal, one, "", bb);
          llvm::Value* sInst = new llvm::StoreInst(inc, location, bb);
          return inc;
        }
        if( op->IsOp("--") ){
          //subtracting one
          llvm::Value *dec = llvm::BinaryOperator::CreateSub(oldVal, one, "", bb);
          llvm::Value* sInst = new llvm::StoreInst(dec, location, bb);
          return dec;
        }
        if( op->IsOp("+") ){
          //llvm::Value* sInst = new llvm::StoreInst(oldVal, location, bb);
          return oldVal;
        }
        if( op->IsOp("-") ){
          llvm::Value* mul = llvm::BinaryOperator::CreateMul(oldVal, negOne, "", bb);
         // llvm::Value* sInst = new llvm::StoreInst(mul, location, bb);
          return mul;
        }
      }
      //Float case
      else if( oldVal->getType() == irgen->GetFloatType() ){
        llvm::Type* type = irgen->GetFloatType();
        llvm::Constant* one = llvm::ConstantFP::get(type,1.0);
        if( op->IsOp("++") ){
          //adding one
          llvm::Value *inc = llvm::BinaryOperator::CreateFAdd(oldVal, one, "", bb);
          llvm::Value* sInst = new llvm::StoreInst(inc, location, bb);
          return inc;
        }
        if( op->IsOp("--") ){
          //subtracting one
          llvm::Value *dec = llvm::BinaryOperator::CreateFSub(oldVal, one, "", bb);
          llvm::Value* sInst = new llvm::StoreInst(dec, location, bb);
          return dec;
        }
        if( op->IsOp("+") ){
          //llvm::Value* sInst = new llvm::StoreInst(oldVal, location, bb);
          return oldVal;
        }
        if( op->IsOp("-") ){
          //llvm::Value* sInst = new llvm::StoreInst(oldVal, location, bb);
          return oldVal;
        }
      }

  }

  //Standard two piece arithmetic expressions
  //Setup
  llvm::Value *lhs = this->left->Emit();
  llvm::Value *rhs = this->right->Emit();
  llvm::BasicBlock *bb = irgen->GetBasicBlock();

  //dealing with arithmetic between floats 
  if( lhs->getType() == irgen->GetType(Type::floatType) &&
      rhs->getType() == irgen->GetType(Type::floatType) ){
    if( op->IsOp("+") ){
      return llvm::BinaryOperator::CreateFAdd(lhs, rhs, "", bb);
    }
    else if( op->IsOp("-") ){
      return llvm::BinaryOperator::CreateFSub(lhs, rhs, "", bb);
    }
    else if( op->IsOp("*") ){
      return llvm::BinaryOperator::CreateFMul(lhs, rhs, "", bb);
    }
    else if( op->IsOp("/") ){
      return llvm::BinaryOperator::CreateFDiv(lhs, rhs, "", bb);
    }
    else{
      return NULL;
    }      
  }
  //dealing with ints only
  else if( lhs->getType() == irgen->GetType(Type::intType) && 
           rhs->getType() == irgen->GetType(Type::intType) ){
    if( op->IsOp("+") ){
      return llvm::BinaryOperator::CreateAdd(lhs, rhs, "", bb);
    }
    else if( op->IsOp("-") ){
      return llvm::BinaryOperator::CreateSub(lhs, rhs, "", bb);
    }
    else if( op->IsOp("*") ){
      return llvm::BinaryOperator::CreateMul(lhs, rhs, "", bb);
    }
    else if( op->IsOp("/") ){
      return llvm::BinaryOperator::CreateSDiv(lhs, rhs, "", bb);
    }
    else{
      return NULL;
    }
  }
  
  //arithmetic between vector and scalar
  else if( (lhs->getType() == irgen->GetType(Type::floatType)) ||
           (rhs->getType() == irgen->GetType(Type::floatType)) ){
    llvm::Value* vec = NULL;
    llvm::Value* constFP = NULL;
    llvm::Type* ty = NULL;
    int vecNum = 0;

    if(lhs->getType() == irgen->GetType(Type::vec2Type)){
      vecNum = 2;
      vec = lhs;
      constFP = rhs;
      ty = irgen->GetType(Type::vec2Type);
    }
    else if(rhs->getType() == irgen->GetType(Type::vec2Type)){
      vecNum = 2;
      vec = rhs;
      constFP = lhs;
      ty = irgen->GetType(Type::vec2Type);
    }
    else if(lhs->getType() == irgen->GetType(Type::vec3Type)){
      vecNum = 3;
      vec = lhs;
      constFP = rhs;
      ty = irgen->GetType(Type::vec3Type);
    }
    else if(rhs->getType() == irgen->GetType(Type::vec3Type)){
      vecNum = 3;
      vec = rhs;
      constFP = lhs;
      ty = irgen->GetType(Type::vec3Type);
    }
    else if(lhs->getType() == irgen->GetType(Type::vec4Type)){
      vecNum = 4;
      vec = lhs;
      constFP = rhs;
      ty = irgen->GetType(Type::vec4Type);
    }
    else if(rhs->getType() == irgen->GetType(Type::vec4Type)){
      vecNum = 4;
      vec = rhs;
      constFP = lhs;
      ty = irgen->GetType(Type::vec4Type);
    }
    llvm::Value *emptyVec = llvm::Constant::getNullValue(ty); //getting empty vec2Type 

    //loading vector and making it a vector type
    llvm::VectorType* vector = dynamic_cast<llvm::VectorType*>(vec);

    //looping through elements in vector
    for(int i = 0; i < vecNum; i++){
      llvm::Value* num = llvm::ConstantInt::get(irgen->GetIntType(), i);
      //inserting element into the empty vector
      emptyVec = llvm::InsertElementInst::Create(emptyVec, constFP, num, "emptyVec", irgen->GetBasicBlock());
    }
    llvm::Value* load = vec;

    //adding between vector and scalar
    if( op->IsOp("+") ){
      return llvm::BinaryOperator::CreateFAdd(emptyVec, load, "", bb);
    }
    //subtracting between vector and scalar
    else if( op->IsOp("-") ){
      return llvm::BinaryOperator::CreateFSub(emptyVec, load, "", bb);
    }
    //multiplying between vector and scalar
    else if( op->IsOp("*") ){
      return llvm::BinaryOperator::CreateFMul(emptyVec, load, "", bb);
    }
    //dividing between vector and scalar
    else if( op->IsOp("/") ){
      return llvm::BinaryOperator::CreateFDiv(emptyVec, load, "", bb);
    }
    else{
      return NULL;
    }
  }
  return NULL;
}

/******** Equality Expr Emit **********/
llvm::Value* EqualityExpr::Emit(){
  //Set up
  llvm::Value *lhs = this->left->Emit();
  llvm::Value *rhs = this->right->Emit();
  llvm::BasicBlock *bb = irgen->GetBasicBlock();
 
  //Check float types
  if( (lhs->getType() == irgen->GetType(Type::floatType)) && (rhs->getType() == irgen->GetType(Type::floatType)) ){
    if( this->op->IsOp("==") ){
      return llvm::CmpInst::Create(llvm::CmpInst::FCmp, llvm::FCmpInst::FCMP_OEQ, lhs, rhs, "", bb);
    }
    if( this->op->IsOp("!=") ){
      return llvm::CmpInst::Create(llvm::CmpInst::FCmp, llvm::FCmpInst::FCMP_ONE, lhs, rhs, "", bb);
    }
  }
  
  //Check int types
  if( (lhs->getType() == irgen->GetType(Type::intType)) && (rhs->getType() == irgen->GetType(Type::intType)) ){
    if( this->op->IsOp("==") ){
      return llvm::CmpInst::Create(llvm::CmpInst::ICmp, llvm::ICmpInst::ICMP_EQ, lhs, rhs, "", bb);
    }
    if( this->op->IsOp("!=") ){
      return llvm::CmpInst::Create(llvm::CmpInst::ICmp, llvm::ICmpInst::ICMP_NE, lhs, rhs, "", bb);
    }
  }
  return NULL;
}
  
/********* Relational Expr Emit **********/
llvm::Value* RelationalExpr::Emit(){
  llvm::Value *lhs = this->left->Emit();
  llvm::Value *rhs = this->right->Emit();
  llvm::BasicBlock *bb = irgen->GetBasicBlock();

  //comparing float types
  if( (lhs->getType() == irgen->GetType(Type::floatType)) && (rhs->getType() == irgen->GetType(Type::floatType)) ){
    if( this->op->IsOp(">") ){
      return llvm::CmpInst::Create(llvm::CmpInst::FCmp, llvm::FCmpInst::FCMP_OGT, lhs, rhs, "", bb);
    }
    if( this->op->IsOp("<") ){
      return llvm::CmpInst::Create(llvm::CmpInst::FCmp, llvm::FCmpInst::FCMP_OLT, lhs, rhs, "", bb);
    }
    if( this->op->IsOp(">=") ){
      return llvm::CmpInst::Create(llvm::CmpInst::FCmp, llvm::FCmpInst::FCMP_OGE, lhs, rhs, "", bb);
    }
    if( this->op->IsOp("<=") ){
      return llvm::CmpInst::Create(llvm::CmpInst::FCmp, llvm::FCmpInst::FCMP_OLE, lhs, rhs, "", bb);
    }
  }

  //comparing int types
  if( (lhs->getType() == irgen->GetType(Type::intType)) && (rhs->getType() == irgen->GetType(Type::intType)) ){
    if( this->op->IsOp(">") ){
      return llvm::CmpInst::Create(llvm::CmpInst::ICmp, llvm::ICmpInst::ICMP_SGT, lhs, rhs, "", bb);
    }
    if( this->op->IsOp("<") ){
      return llvm::CmpInst::Create(llvm::CmpInst::ICmp, llvm::ICmpInst::ICMP_SLT, lhs, rhs, "", bb);
    }
    if( this->op->IsOp(">=") ){
      return llvm::CmpInst::Create(llvm::CmpInst::ICmp, llvm::ICmpInst::ICMP_SGE, lhs, rhs, "", bb);
    }
    if( this->op->IsOp("<=") ){
      return llvm::CmpInst::Create(llvm::CmpInst::ICmp, llvm::ICmpInst::ICMP_SLE, lhs, rhs, "", bb);
    }
  }
  return NULL;
}


/********** Conditional Expr Emit *********/
llvm::Value* ConditionalExpr::Emit(){
  //llvm::SelectInst::Create(Value *condition, Value *trueValue, Value *falseValue, const Twine &NameStr, BasicBlock *InsertAtEnd );
  llvm::BasicBlock *bb = irgen->GetBasicBlock();
  //IfStmt* ifstmt = new Ifstmt(this->cond, this->trueExpr, this->falseExpr);
  llvm::Value* cond = this->cond->Emit();
  llvm::Value* trueVal = this->trueExpr->Emit();
  llvm::Value* falseVal = this->falseExpr->Emit();

  //@1524

  return llvm::SelectInst::Create(cond, trueVal, falseVal, "Selection", bb);
}


/********** Logical Expr Emit ***********/
llvm::Value* LogicalExpr::Emit(){
  //llvm::BinaryOperator::CreateAnd(Value *S1, Value *S2, const Twine &Name, BasicBlock *InsertAtEnd)
  //llvm::BinaryOperator::CreateOr(Value *S1, Value *S2, const Twine &Name, BasicBlock *InsertAtEnd)
  //Setup
  llvm::Value *lhs = this->left->Emit();
  llvm::Value *rhs = this->right->Emit();
  llvm::BasicBlock *bb = irgen->GetBasicBlock();

  if( this->op->IsOp("&&") ){
    return llvm::BinaryOperator::CreateAnd(lhs, rhs, "LogicalAnd", bb);
  }
  if( this->op->IsOp("||") ){
    return llvm::BinaryOperator::CreateOr(lhs, rhs, "LogicalOr", bb);
  }
  return NULL;
}


/********* Postfix Expr Emit *********/
llvm::Value* PostfixExpr::Emit(){
    //getting the value at the pointer
    llvm::Value *oldVal = this->left->Emit(); //"loading from pointer"

    //getting the pointer of the value
    llvm::LoadInst *l = llvm::cast<llvm::LoadInst>(oldVal);
    llvm::Value *location = l->getPointerOperand(); //getting pointer?

    //getting the basic block
    llvm::BasicBlock *bb = irgen->GetBasicBlock();

    //Int Post fix
    if( oldVal->getType() == irgen->GetType(Type::intType)){
        //Get a useful '1'
        llvm::Type *intTy = irgen->GetIntType();
        llvm::Value *one = llvm::ConstantInt::get(intTy, 1);
        //post dec
        if( this->op->IsOp("--") ){
            //creating binary op
            llvm::Value *dec = llvm::BinaryOperator::CreateSub(oldVal, one, "", bb);
            //storing new value
            llvm::Value* sInst = new llvm::StoreInst(dec, location, bb);
        }
        if( this->op->IsOp("++") ){
            //creating binary op
            llvm::Value *inc = llvm::BinaryOperator::CreateAdd(oldVal, one, "", bb);
            //storing new value
            llvm::Value* sInst = new llvm::StoreInst(inc, location, bb);
        }
    }
    //Float Post fix
    else if( oldVal->getType() == irgen->GetType(Type::floatType)){
       //Get a useful '1'
        llvm::Type* type = irgen->GetFloatType();
        llvm::Constant* one = llvm::ConstantFP::get(type,1.0);
        //post dec
        if( this->op->IsOp("--") ){
            //creating binary op
            llvm::Value *dec = llvm::BinaryOperator::CreateFSub(oldVal, one, "", bb);
            //storing new value
            llvm::Value* sInst = new llvm::StoreInst(dec, location, bb);
        }
        //Post inc
        if( this->op->IsOp("++") ){
            //creating binary op
            llvm::Value *inc = llvm::BinaryOperator::CreateFAdd(oldVal, one, "", bb);
            //storing new value
            llvm::Value* sInst = new llvm::StoreInst(inc, location, bb);
        }
    }
    return oldVal;
}

/********** Assign Expr Emit **********/
llvm::Value* AssignExpr::Emit(){
  Operator *op = this->op;
  llvm::Value *rVal = this->right->Emit();
  llvm::Value *lVal = this->left->Emit();
  llvm::LoadInst* leftLocation = llvm::cast<llvm::LoadInst>(lVal);

  llvm::ShuffleVectorInst *shuffleLHS = dynamic_cast<llvm::ShuffleVectorInst*>(this->left->Emit());
  llvm::ShuffleVectorInst *shuffleRHS = dynamic_cast<llvm::ShuffleVectorInst*>(this->right->Emit());

    //Regular assignment
  if(this->op->IsOp("=")){
      if( (shuffleLHS != NULL) && (shuffleRHS != NULL) ){
        
      }
      else if( (shuffleLHS != NULL) && 
               ((rVal->getType() == irgen->GetType(Type::vec2Type)) || 
                (rVal->getType() == irgen->GetType(Type::vec3Type)) ||
                (rVal->getType() == irgen->GetType(Type::vec4Type)) )){
        //rhs is a vector
        //cout << "whaa"<<endl;
        FieldAccess *f = dynamic_cast<FieldAccess*>(this->left);
        llvm::Value* base = f->getBase()->Emit();
        llvm::LoadInst* l = llvm::cast<llvm::LoadInst>(base);
        char*field = f->getField()->GetName();
        //llvm::LoadInst* l = new llvm::LoadInst(lVal, "", irgen->GetBasicBlock());
        llvm::Value* vec= base;
        /*if(strlen(field) == 2){
          vec = llvm::Constant::getNullValue(irgen->GetType(Type::vec2Type));
        }
        else if(strlen(field) == 3){
          vec = llvm::Constant::getNullValue(irgen->GetType(Type::vec3Type));
        }
        else if(strlen(field) == 4){
          vec = llvm::Constant::getNullValue(irgen->GetType(Type::vec4Type));
        }*/
        int count = 0;
        llvm::Value* leftIdx = NULL;
        llvm::Value* rightIdx = NULL;
        for(char*it = field; *it; ++it){
          if(*it == 'x'){
            leftIdx = llvm::ConstantInt::get(irgen->GetIntType(), 0);
            rightIdx = llvm::ConstantInt::get(irgen->GetIntType(), count);
            llvm::Value* newElmt = llvm::ExtractElementInst::Create( rVal, rightIdx, "", irgen->GetBasicBlock() );
            vec = llvm::InsertElementInst::Create( vec, newElmt, leftIdx, "", irgen->GetBasicBlock() );
          }
          else if(*it == 'y'){
            leftIdx = llvm::ConstantInt::get(irgen->GetIntType(), 1);
            rightIdx = llvm::ConstantInt::get(irgen->GetIntType(), count);
            llvm::Value* newElmt = llvm::ExtractElementInst::Create( rVal, rightIdx, "", irgen->GetBasicBlock() );
            vec = llvm::InsertElementInst::Create( vec, newElmt, leftIdx, "", irgen->GetBasicBlock() );
          }
          else if(*it == 'z'){
            leftIdx = llvm::ConstantInt::get(irgen->GetIntType(), 2);
            rightIdx = llvm::ConstantInt::get(irgen->GetIntType(), count);
            llvm::Value* newElmt = llvm::ExtractElementInst::Create( rVal, rightIdx, "", irgen->GetBasicBlock() );
            vec = llvm::InsertElementInst::Create( vec, newElmt, leftIdx, "", irgen->GetBasicBlock() );
          }
          else if(*it == 'w'){
            leftIdx = llvm::ConstantInt::get(irgen->GetIntType(), 3);
            rightIdx = llvm::ConstantInt::get(irgen->GetIntType(), count);
            llvm::Value* newElmt = llvm::ExtractElementInst::Create( rVal, rightIdx, "", irgen->GetBasicBlock() );
            vec = llvm::InsertElementInst::Create( vec, newElmt, leftIdx, "", irgen->GetBasicBlock() );
          }
          count++;
        }
        new llvm::StoreInst(vec, l->getPointerOperand(), irgen->GetBasicBlock());
      }
      /*else if( (dynamic_cast<llvm::ExtractElementInst*>(lVal) != NULL) &&
               (dynamic_cast<llvm::ExtractElementInst*>(rVal) != NULL) ){
        //assigning swizzle to swizzle

      }*/
      else if( dynamic_cast<llvm::ExtractElementInst*>(lVal) != NULL ){
        //cout << "HI" << endl;
        //assigning to single swizzle
        FieldAccess *f = dynamic_cast<FieldAccess*>(this->left);
        llvm::Value* base = f->getBase()->Emit();
        llvm::LoadInst* l = llvm::cast<llvm::LoadInst>(base);

        char*field = f->getField()->GetName();
        llvm::Value* vec= base;

        int count = 0;
        llvm::Value* leftIdx = NULL;
        //llvm::Value* rightIdx = NULL;
        char*it = f->getField()->GetName();
        if(*it == 'x'){
          leftIdx = llvm::ConstantInt::get(irgen->GetIntType(), 0);
          vec = llvm::InsertElementInst::Create( vec,rVal, leftIdx, "", irgen->GetBasicBlock() );
        }
        else if(*it == 'y'){
          leftIdx = llvm::ConstantInt::get(irgen->GetIntType(), 1);
          vec = llvm::InsertElementInst::Create( vec, rVal, leftIdx, "", irgen->GetBasicBlock() );
        }
        else if(*it == 'z'){
          leftIdx = llvm::ConstantInt::get(irgen->GetIntType(), 2);
          vec = llvm::InsertElementInst::Create( vec, rVal, leftIdx, "", irgen->GetBasicBlock() );
        }
        else if(*it == 'w'){
          leftIdx = llvm::ConstantInt::get(irgen->GetIntType(), 3);
          vec = llvm::InsertElementInst::Create( vec, rVal, leftIdx, "", irgen->GetBasicBlock() );
        }
        new llvm::StoreInst(vec, l->getPointerOperand(), irgen->GetBasicBlock());
      }
      else{
        //cout << "YO" << endl;
        new llvm::StoreInst(rVal, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
      }
      return rVal;
    }
    //Float assignments
    else if( ((lVal->getType() == irgen->GetType(Type::floatType)) && (rVal->getType() == irgen->GetType(Type::floatType))) /*||
      ((lVal->getType() == irgen->GetType(Type::vec2Type)) && (rVal->getType() == irgen->GetType(Type::vec2Type)))*/ ){
      if( this->op->IsOp("*=") ){
        llvm::Value *mul = llvm::BinaryOperator::CreateFMul(lVal, rVal, "mulequal", irgen->GetBasicBlock());
        llvm::Value *sInst = new llvm::StoreInst(mul, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
        return mul;
      }
      else if( this->op->IsOp("+=") ){
        llvm::Value *add = llvm::BinaryOperator::CreateFAdd(lVal, rVal, "plusequal", irgen->GetBasicBlock());
        llvm::Value *sInst = new llvm::StoreInst(add, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
        return add;
      }
      else if( this->op->IsOp("-=") ){
        llvm::Value *min = llvm::BinaryOperator::CreateFSub(lVal, rVal, "minusequal", irgen->GetBasicBlock());
        llvm::Value *sInst = new llvm::StoreInst(min, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
        return min;
      }
      else if( this->op->IsOp("/=") ){
        llvm::Value *div = llvm::BinaryOperator::CreateFDiv(lVal, rVal, "divequal", irgen->GetBasicBlock());
        llvm::Value *sInst = new llvm::StoreInst(div, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
        return div;
      }
  }
    //Int assignments
    else if( (lVal->getType() == irgen->GetType(Type::intType)) && (rVal->getType() == irgen->GetType(Type::intType)) ){
      if( this->op->IsOp("*=") ){
        llvm::Value *mul = llvm::BinaryOperator::CreateMul(lVal, rVal, "mulequal", irgen->GetBasicBlock());
        llvm::Value *sInst = new llvm::StoreInst(mul, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
        return mul;
      }
      else if( this->op->IsOp("+=") ){
        llvm::Value *add = llvm::BinaryOperator::CreateAdd(lVal, rVal, "plusequal", irgen->GetBasicBlock());
        llvm::Value *sInst = new llvm::StoreInst(add, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
        return add;
      }
      else if( this->op->IsOp("-=") ){
        llvm::Value *min = llvm::BinaryOperator::CreateSub(lVal, rVal, "minusequal", irgen->GetBasicBlock());
        llvm::Value *sInst = new llvm::StoreInst(min, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
        return min;
      }
      else if( this->op->IsOp("/=") ){
        llvm::Value *div = llvm::BinaryOperator::CreateSDiv(lVal, rVal, "divequal", irgen->GetBasicBlock());
        llvm::Value *sInst = new llvm::StoreInst(div, leftLocation->getPointerOperand(), irgen->GetBasicBlock());
        return div;
      }
    }
    return NULL;
    //return new llvm::LoadInst(lVal, "", irgen->GetBasicBlock());
    //return leftLocation;
}

/********** ArrayAccess Emit ***********/
llvm::Value* ArrayAccess::Emit(){
  //llvm::GetElementPtrInst::Create(Value *Ptr, ArrayRef<Value*> IdxList, const Twine &NameStr, BasicBlock *InsertAtEnd);
  //idx = llvm::ConstantInt::get(irgen->GetIntType(), 0);
  std::vector<llvm::Value*> arrayBase;
  arrayBase.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
  arrayBase.push_back(subscript->Emit());
  llvm::Value* arrayElem = llvm::GetElementPtrInst::Create(dynamic_cast<llvm::LoadInst*>(base->Emit())->getPointerOperand(), arrayBase, "", irgen->GetBasicBlock());
  //llvm::Value* lInst = new llvm::LoadInst( v, exprName, irgen->GetBasicBlock() );
  return new llvm::LoadInst(arrayElem, "", irgen->GetBasicBlock());
}

/********* Field Access Emit **********/
llvm::Value* FieldAccess::Emit(){
  if( this->base != NULL ){
    llvm::Value* base = this->base->Emit();
    llvm::BasicBlock* bb = irgen->GetBasicBlock();
    
    std::vector<llvm::Constant*> swizzles;
    if( this->field != NULL ){
      char* f = this->field->GetName();
      llvm::Constant* idx;
      for(char* it = f; *it; ++it){
        if(*it == 'x')
          idx = llvm::ConstantInt::get(irgen->GetIntType(), 0);
        else if(*it == 'y')
          idx = llvm::ConstantInt::get(irgen->GetIntType(), 1);
        else if(*it == 'z')
          idx = llvm::ConstantInt::get(irgen->GetIntType(), 2);
        else if(*it == 'w')
          idx = llvm::ConstantInt::get(irgen->GetIntType(), 3);
        else
          idx = llvm::ConstantInt::get(irgen->GetIntType(), 100);//impossible
        swizzles.push_back(idx);
      }
      if(strlen(f) < 2){
        return llvm::ExtractElementInst::Create(base, idx, "", bb);
      }
      llvm::ArrayRef<llvm::Constant*> swizzleArrayRef(swizzles);
      llvm::Constant* mask = llvm::ConstantVector::get(swizzleArrayRef);
      llvm::Value* newVec = new llvm::ShuffleVectorInst(base, base, mask, "", bb);
      
      return newVec;
    }
  }
  /*
  if( this->base != NULL){
    llvm::Value* base = this->base->Emit();
    llvm::BasicBlock* bb = irgen->GetBasicBlock();
    //llvm::LoadInst *l = llvm::cast<llvm::LoadInst>(base);
    //llvm::Value *location = l->getPointerOperand();

    llvm::Value *idx = NULL;
    string x = "x", y = "y", z = "z", w = "w";
    //for
    if(this->field->GetName() == x){
      idx = llvm::ConstantInt::get(irgen->GetIntType(), 0);
    }
    else if(this->field->GetName() == y){
      idx = llvm::ConstantInt::get(irgen->GetIntType(), 1);
    }
    else if(this->field->GetName() == z){
      idx = llvm::ConstantInt::get(irgen->GetIntType(), 2);
    }
    else if(this->field->GetName() == w){
      idx = llvm::ConstantInt::get(irgen->GetIntType(), 3);
    }
    //if (idx == NULL){
    //  cout << "fnjdls" << endl;
    //}
    
    llvm::Value *v = llvm::ExtractElementInst::Create(base, idx, "", bb);
    return v;
  }
  */
  return NULL;
}

/*********** Call Emit ***********/
llvm::Value* Call::Emit(){
  //Used for "Call" expression
  // Func should be the address of the function
  // Args is an ArrayRef<Value*> of the Actuals LLVM::Value
  //llvm::CallInst::Create( Value *Func, ArrayRef<Value*> Args, const Twine &NameStr, BasicBlock *InsertAtEnd );
  //llvm::ArrayRef<llvm::Constant*> swizzleArrayRef(swizzles);
  //idx = llvm::ConstantInt::get(irgen->GetIntType(), 100);
  //std::vector<llvm::Constant*> swizzles;
  llvm::Function* func = (llvm::Function*)symtable->lookup(field->GetName());
  std::vector<llvm::Value*> valActuals;
  for(int x = 0; x < actuals->NumElements(); x++){
    valActuals.push_back(actuals->Nth(x)->Emit());
  }
  
  return llvm::CallInst::Create(func, valActuals, "Call", irgen->GetBasicBlock());
}

/********* Emits for int, float, bool, any Var ***********/
llvm::Value* IntConstant::Emit(){
    llvm::Type *intTy = irgen->GetIntType();
    return llvm::ConstantInt::get(intTy, this->value);
}

llvm::Value* FloatConstant::Emit(){
    llvm::Type *fTy = irgen->GetFloatType();
    return llvm::ConstantFP::get(fTy, this->value);
}

llvm::Value* BoolConstant::Emit(){
    llvm::Type *bTy = irgen->GetBoolType();

    // true == 1, false == 0
    return llvm::ConstantInt::get(bTy, this->value); 
}

llvm::Value* VarExpr::Emit(){
    string exprName = this->GetIdentifier()->GetName();
    llvm::Value* v = symtable->lookup(exprName);
    llvm::Value* lInst = new llvm::LoadInst( v, exprName, irgen->GetBasicBlock() );
    return lInst;
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}
void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

VarExpr::VarExpr(yyltype loc, Identifier *ident) : Expr(loc) {
    Assert(ident != NULL);
    this->id = ident;
}

void VarExpr::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

void Operator::PrintChildren(int indentLevel) {
    printf("%s",tokenString);
}

bool Operator::IsOp(const char *op) const {
    return strcmp(tokenString, op) == 0;
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o) 
  : Expr(Join(l->GetLocation(), o->GetLocation())) {
    Assert(l != NULL && o != NULL);
    (left=l)->SetParent(this);
    (op=o)->SetParent(this);
}

void CompoundExpr::PrintChildren(int indentLevel) {
   if (left) left->Print(indentLevel+1);
   op->Print(indentLevel+1);
   if (right) right->Print(indentLevel+1);
}
   
ConditionalExpr::ConditionalExpr(Expr *c, Expr *t, Expr *f)
  : Expr(Join(c->GetLocation(), f->GetLocation())) {
    Assert(c != NULL && t != NULL && f != NULL);
    (cond=c)->SetParent(this);
    (trueExpr=t)->SetParent(this);
    (falseExpr=f)->SetParent(this);
}

void ConditionalExpr::PrintChildren(int indentLevel) {
    cond->Print(indentLevel+1, "(cond) ");
    trueExpr->Print(indentLevel+1, "(true) ");
    falseExpr->Print(indentLevel+1, "(false) ");
}
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
}
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}


void FieldAccess::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    field->Print(indentLevel+1);
}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

void Call::PrintChildren(int indentLevel) {
   if (base) base->Print(indentLevel+1);
   if (field) field->Print(indentLevel+1);
   if (actuals) actuals->PrintAll(indentLevel+1, "(actuals) ");
}

