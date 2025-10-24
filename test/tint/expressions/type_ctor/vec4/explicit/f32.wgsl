var<private> v = vec4(0.f, 1.f, 2.f, 3.f);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
