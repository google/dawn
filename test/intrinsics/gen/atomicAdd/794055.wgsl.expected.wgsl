var<workgroup> arg_0 : atomic<i32>;

fn atomicAdd_794055() {
  var res : i32 = atomicAdd(&(arg_0), 1);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicAdd_794055();
}
