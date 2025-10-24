var<private> v = vec4(0, 1, 2, 3);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
