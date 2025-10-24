var<private> v = vec3(false, true, false);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
