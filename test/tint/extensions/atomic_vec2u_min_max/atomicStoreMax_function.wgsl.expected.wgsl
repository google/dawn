enable atomic_vec2u_min_max;

@group(0) @binding(0) var<storage, read_write> a : atomic<vec2u>;

fn foo(p : ptr<storage, atomic<vec2u>, read_write>) {
  atomicStoreMin(p, vec2u(0, 0));
}

@fragment
fn main() {
  foo(&(a));
}
