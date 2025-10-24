var<private> u : vec4<bool> = vec4<bool>(vec4<i32>(1i));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
