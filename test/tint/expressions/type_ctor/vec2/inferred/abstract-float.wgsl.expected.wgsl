var<private> v = vec2(0.0, 1.0);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
