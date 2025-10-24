var<private> u : vec4<i32> = vec4<i32>(vec4<bool>(true));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
