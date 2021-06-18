SKIP: FAILED


var<workgroup> arg_0 : atomic<u32>;

fn atomicAnd_34edd3() {
  var res : u32 = atomicAnd(&(arg_0), 1u);
}

[[stage(compute)]]
fn compute_main() {
  atomicAnd_34edd3();
}

Failed to generate: error: unknown type in EmitType
