SKIP: FAILED


var<workgroup> arg_0 : atomic<u32>;

fn atomicMin_69d383() {
  var res : u32 = atomicMin(&(arg_0), 1u);
}

[[stage(compute)]]
fn compute_main() {
  atomicMin_69d383();
}

Failed to generate: error: unknown type in EmitType
