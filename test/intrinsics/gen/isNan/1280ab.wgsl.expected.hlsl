SKIP: FAILED



Validation Failure:
void isNan_1280ab() {
  vector<bool, 3> res = isnan(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  isNan_1280ab();
  return;
}

void fragment_main() {
  isNan_1280ab();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_1280ab();
  return;
}


dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.
