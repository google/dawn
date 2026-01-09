struct buffer_type_atomic {
  tint_symbol : array<atomic<u32>>,
}

@group(0u) @binding(0u) var<storage, read_write> v : buffer_type_atomic;

@compute @workgroup_size(64u, 1u, 1u)
fn main() {
  let v_1 = &(v.tint_symbol[0i]);
  _ = atomicLoad(v_1);
  if (true) {
    _ = atomicCompareExchangeWeak(v_1, 0u, 0u).old_value;
  }
}
