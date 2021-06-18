var<workgroup> arg_0 : atomic<i32>;

fn atomicAdd_794055() {
  var res : i32 = atomicAdd(&(arg_0), 1);
}

[[stage(compute)]]
fn compute_main() {
  atomicAdd_794055();
}
