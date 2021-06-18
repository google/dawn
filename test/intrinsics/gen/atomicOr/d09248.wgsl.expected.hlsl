SKIP: FAILED


var<workgroup> arg_0 : atomic<i32>;

fn atomicOr_d09248() {
  var res : i32 = atomicOr(&(arg_0), 1);
}

[[stage(compute)]]
fn compute_main() {
  atomicOr_d09248();
}

Failed to generate: error: unknown type in EmitType
