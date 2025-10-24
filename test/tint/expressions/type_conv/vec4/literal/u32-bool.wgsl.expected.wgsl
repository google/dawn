var<private> u : vec4<bool> = vec4<bool>(vec4<u32>(1u));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
