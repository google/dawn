var<private> u : vec3<bool> = vec3<bool>(vec3<u32>(1u));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}
