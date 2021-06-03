SKIP: FAILED



Validation Failure:
void isNan_67ecd3() {
  vector<bool, 2> res = isnan(float2(0.0f, 0.0f));
}

void vertex_main() {
  isNan_67ecd3();
  return;
}

void fragment_main() {
  isNan_67ecd3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_67ecd3();
  return;
}


dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.
