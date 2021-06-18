var<workgroup> arg_0 : atomic<u32>;

fn atomicExchange_0a5dca() {
  var res : u32 = atomicExchange(&(arg_0), 1u);
}

[[stage(compute)]]
fn compute_main() {
  atomicExchange_0a5dca();
}
