SKIP: FAILED


var<workgroup> arg_0 : atomic<u32>;

fn atomicCompareExchangeWeak_b2ab2c() {
  var res : vec2<u32> = atomicCompareExchangeWeak(&(arg_0), 1u, 1u);
}

[[stage(compute)]]
fn compute_main() {
  atomicCompareExchangeWeak_b2ab2c();
}

Failed to generate: error: unknown type in EmitType
