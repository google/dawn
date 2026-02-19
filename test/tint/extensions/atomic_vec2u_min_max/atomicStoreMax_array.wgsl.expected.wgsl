enable atomic_vec2u_min_max;

@group(0) @binding(0) var<storage, read_write> sb_rw0 : array<atomic<vec2<u32>>, 4>;

@group(0) @binding(1) var<storage, read_write> sb_rw1 : array<atomic<vec2<u32>>>;

@compute @workgroup_size(1)
fn compute_main() {
  atomicStoreMax(&(sb_rw0[0]), vec2<u32>(1u));
  atomicStoreMax(&(sb_rw1[0]), vec2<u32>(1u));
}
