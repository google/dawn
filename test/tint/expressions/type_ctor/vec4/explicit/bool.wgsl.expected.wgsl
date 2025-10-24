var<private> v = vec4(false, true, false, true);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
