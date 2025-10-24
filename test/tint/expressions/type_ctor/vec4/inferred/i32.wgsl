var<private> v = vec4(0i, 1i, 2i, 3i);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
