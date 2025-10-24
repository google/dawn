var<private> v = vec4(0.0f, 1.0f, 2.0f, 3.0f);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
