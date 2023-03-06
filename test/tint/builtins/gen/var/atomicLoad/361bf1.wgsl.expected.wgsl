var<workgroup> arg_0 : atomic<u32>;

fn atomicLoad_361bf1() {
  var res : u32 = atomicLoad(&(arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicLoad_361bf1();
}
