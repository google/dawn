var<workgroup> arg_0 : atomic<u32>;

fn atomicMin_69d383() {
  var res : u32 = atomicMin(&(arg_0), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicMin_69d383();
}
