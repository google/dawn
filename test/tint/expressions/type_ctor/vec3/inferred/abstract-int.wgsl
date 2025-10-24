var<private> v = vec3(0, 1, 2);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
