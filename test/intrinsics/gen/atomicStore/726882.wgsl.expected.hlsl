SKIP: FAILED


var<workgroup> arg_0 : atomic<u32>;

fn atomicStore_726882() {
  atomicStore(&(arg_0), 1u);
}

[[stage(compute)]]
fn compute_main() {
  atomicStore_726882();
}

Failed to generate: error: unknown type in EmitType
