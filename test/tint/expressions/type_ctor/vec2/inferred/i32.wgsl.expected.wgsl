var<private> v = vec2(0i, 1i);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
