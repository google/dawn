struct S {
    a: array<mat3x3<f32>, 3>,
    b: array<array<vec3<f32>, 3>, 3>,
}

@group(0) @binding(0)
var<storage, read_write> s: S;

@compute @workgroup_size(1u)
fn main() {
    s = s;
}
