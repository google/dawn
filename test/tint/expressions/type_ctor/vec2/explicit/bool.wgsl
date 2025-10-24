var<private> v = vec2<bool>(false, true);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
