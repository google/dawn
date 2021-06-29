var<workgroup> arg_0 : atomic<u32>;

fn atomicLoad_361bf1() {
  var res : u32 = atomicLoad(&(arg_0));
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicLoad_361bf1();
}
