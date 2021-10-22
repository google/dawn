// Check that for backends that generate intrinsic helpers, repeated use of the
// same intrinsic overload results in single helper being generated.
[[stage(compute), workgroup_size(1)]]
fn main() {
    _ = isNormal(vec4<f32>());
    _ = isNormal(vec4<f32>(1.));
    _ = isNormal(vec4<f32>(1., 2., 3., 4.));

    _ = isNormal(vec3<f32>());
    _ = isNormal(vec3<f32>(1.));
    _ = isNormal(vec3<f32>(1., 2., 3.));

    _ = isNormal(vec2<f32>());
    _ = isNormal(vec2<f32>(1.));
    _ = isNormal(vec2<f32>(1., 2.));

    _ = isNormal(1.);
    _ = isNormal(2.);
    _ = isNormal(3.);
}
