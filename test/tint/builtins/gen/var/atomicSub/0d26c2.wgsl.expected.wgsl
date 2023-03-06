var<workgroup> arg_0 : atomic<u32>;

fn atomicSub_0d26c2() {
  var arg_1 = 1u;
  var res : u32 = atomicSub(&(arg_0), arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicSub_0d26c2();
}
