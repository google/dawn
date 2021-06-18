SKIP: FAILED


var<workgroup> arg_0 : atomic<u32>;

fn atomicMax_beccfc() {
  var res : u32 = atomicMax(&(arg_0), 1u);
}

[[stage(compute)]]
fn compute_main() {
  atomicMax_beccfc();
}

Failed to generate: error: unknown type in EmitType
