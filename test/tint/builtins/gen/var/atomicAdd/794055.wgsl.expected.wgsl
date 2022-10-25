var<workgroup> arg_0 : atomic<i32>;

fn atomicAdd_794055() {
  var arg_1 = 1i;
  var res : i32 = atomicAdd(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicAdd_794055();
}
