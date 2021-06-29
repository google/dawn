var<workgroup> arg_0 : atomic<i32>;

fn atomicExchange_e114ba() {
  var res : i32 = atomicExchange(&(arg_0), 1);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicExchange_e114ba();
}
