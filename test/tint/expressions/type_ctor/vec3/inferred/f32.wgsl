var<private> v = vec3(0.0f, 1.0f, 2.0f);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
