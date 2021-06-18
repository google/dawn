SKIP: FAILED


var<workgroup> arg_0 : atomic<u32>;

fn atomicOr_5e3d61() {
  var res : u32 = atomicOr(&(arg_0), 1u);
}

[[stage(compute)]]
fn compute_main() {
  atomicOr_5e3d61();
}

Failed to generate: error: unknown type in EmitType
