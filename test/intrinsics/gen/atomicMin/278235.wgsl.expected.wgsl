var<workgroup> arg_0 : atomic<i32>;

fn atomicMin_278235() {
  var res : i32 = atomicMin(&(arg_0), 1);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicMin_278235();
}
