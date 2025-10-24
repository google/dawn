var<private> u : vec3<f32> = vec3<f32>(vec3<i32>(1i));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}
