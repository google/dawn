var<workgroup> arg_0 : atomic<i32>;

fn atomicExchange_e114ba() {
  var arg_1 = 1i;
  var res : i32 = atomicExchange(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicExchange_e114ba();
}
