enable atomic_vec2u_min_max;

@group(0) @binding(0) var<storage, read_write> a : array<atomic<vec2u>>;

@group(0) @binding(1) var<storage, read> other : u32;

@fragment
fn main() {
  atomicStoreMin(&(a[0]), vec2u(0, 0));
}
