enable atomic_vec2u_min_max;

struct SB_RW {
  arg_0 : atomic<vec2<u32>>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicStoreMax_707bde() {
  var arg_1 = vec2<u32>(1u);
  atomicStoreMax(&(sb_rw.arg_0), arg_1);
}

@fragment
fn fragment_main() {
  atomicStoreMax_707bde();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicStoreMax_707bde();
}
