struct buffer_type_atomic {
  tint_symbol : array<atomic<u32>>,
}

@group(0u) @binding(0u) var<storage, read_write> buffer : buffer_type_atomic;

@compute @workgroup_size(64u, 1u, 1u)
fn main() {
  let v = &(buffer.tint_symbol[0i]);
  _ = atomicLoad(v);
  if (true) {
    _ = atomicCompareExchangeWeak(v, 0u, 0u).old_value;
  }
}
