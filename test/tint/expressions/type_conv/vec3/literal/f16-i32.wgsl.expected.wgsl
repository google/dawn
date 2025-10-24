enable f16;

var<private> u : vec3<i32> = vec3<i32>(vec3<f16>(1.0h));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
