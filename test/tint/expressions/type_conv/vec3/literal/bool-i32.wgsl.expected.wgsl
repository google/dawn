var<private> u : vec3<i32> = vec3<i32>(vec3<bool>(true));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
