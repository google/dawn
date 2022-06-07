var<workgroup> arg_0 : atomic<u32>;

fn atomicAdd_d5db1d() {
  var arg_1 = 1u;
  var res : u32 = atomicAdd(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicAdd_d5db1d();
}
