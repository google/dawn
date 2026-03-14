enable atomic_vec2u_min_max;

@group(0) @binding(0) var<storage, read_write> a : atomic<vec2u>;

@fragment
fn main() {
  let pa = &(a);
  atomicStoreMax(pa, vec2u(0, 0));
}
