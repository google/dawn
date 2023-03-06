struct SB_RW {
  arg_0 : atomic<u32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicMin_c67a74() {
  var res : u32 = atomicMin(&(sb_rw.arg_0), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@fragment
fn fragment_main() {
  atomicMin_c67a74();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicMin_c67a74();
}
