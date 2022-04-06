// Check that for backends that generate builtin helpers, repeated use of the
// same builtin overload results in single helper being generated.
@stage(compute) @workgroup_size(1)
fn main() {
    _ = degrees(vec4<f32>());
    _ = degrees(vec4<f32>(1.));
    _ = degrees(vec4<f32>(1., 2., 3., 4.));

    _ = degrees(vec3<f32>());
    _ = degrees(vec3<f32>(1.));
    _ = degrees(vec3<f32>(1., 2., 3.));

    _ = degrees(vec2<f32>());
    _ = degrees(vec2<f32>(1.));
    _ = degrees(vec2<f32>(1., 2.));

    _ = degrees(1.);
    _ = degrees(2.);
    _ = degrees(3.);
}
