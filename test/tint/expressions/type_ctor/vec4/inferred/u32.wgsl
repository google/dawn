var<private> v = vec4(0u, 1u, 2u, 3u);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
