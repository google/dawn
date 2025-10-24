var<private> v = vec3<i32>(0i, 1i, 2i);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
