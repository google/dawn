var<private> v = vec3<u32>(0u, 1u, 2u);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
