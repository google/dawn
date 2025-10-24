var<private> v = vec3(0.0, 1.0, 2.0);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
