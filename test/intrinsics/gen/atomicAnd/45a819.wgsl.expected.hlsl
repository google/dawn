SKIP: FAILED


var<workgroup> arg_0 : atomic<i32>;

fn atomicAnd_45a819() {
  var res : i32 = atomicAnd(&(arg_0), 1);
}

[[stage(compute)]]
fn compute_main() {
  atomicAnd_45a819();
}

Failed to generate: error: unknown type in EmitType
