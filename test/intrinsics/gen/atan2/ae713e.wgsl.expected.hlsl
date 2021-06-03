SKIP: FAILED



Validation Failure:
void atan2_ae713e() {
  float4 res = atan2(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  atan2_ae713e();
  return;
}

void fragment_main() {
  atan2_ae713e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_ae713e();
  return;
}


dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.

dxc: /build/directxshadercompiler-1.4.0.2274-413-1lunarg18.04/include/llvm/Support/Casting.h:238: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*) [with X = llvm::ConstantFP; Y = llvm::Value; typename llvm::cast_retty<X, Y*>::ret_type = llvm::ConstantFP*]: Assertion `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.
