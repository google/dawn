var<private> v = vec2<f32>(0.0f, 1.0f);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
