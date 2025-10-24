var<private> v = vec2<u32>(0u, 1u);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
