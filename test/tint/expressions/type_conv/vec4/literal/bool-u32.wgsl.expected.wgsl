var<private> u : vec4<u32> = vec4<u32>(vec4<bool>(true));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
