var<workgroup> arg_0 : atomic<u32>;

fn atomicAnd_34edd3() {
  var res : u32 = atomicAnd(&(arg_0), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicAnd_34edd3();
}
