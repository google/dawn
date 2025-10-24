enable f16;

var<private> v = vec3(0.0h, 1.0h, 2.0h);

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
