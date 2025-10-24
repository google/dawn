var<private> v = vec2(0u, 1u);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
