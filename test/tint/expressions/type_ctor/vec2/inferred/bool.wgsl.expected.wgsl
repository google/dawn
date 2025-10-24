var<private> v = vec2(false, true);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
