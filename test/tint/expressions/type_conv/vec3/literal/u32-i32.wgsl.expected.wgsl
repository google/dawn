var<private> u : vec3<i32> = vec3<i32>(vec3<u32>(1u));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
