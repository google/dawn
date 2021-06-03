SKIP: FAILED



Validation Failure:
void atan2_a70d0d() {
  float3 res = atan2(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  atan2_a70d0d();
  return;
}

void fragment_main() {
  atan2_a70d0d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_a70d0d();
  return;
}


dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.
