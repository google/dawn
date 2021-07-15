// Check that for backends that generate intrinsic helpers, repeated use of the
// same intrinsic overload results in single helper being generated.
[[stage(compute), workgroup_size(1)]]
fn main() {
    ignore(isNormal(vec4<f32>()));
    ignore(isNormal(vec4<f32>(1.)));
    ignore(isNormal(vec4<f32>(1., 2., 3., 4.)));

    ignore(isNormal(vec3<f32>()));
    ignore(isNormal(vec3<f32>(1.)));
    ignore(isNormal(vec3<f32>(1., 2., 3.)));

    ignore(isNormal(vec2<f32>()));
    ignore(isNormal(vec2<f32>(1.)));
    ignore(isNormal(vec2<f32>(1., 2.)));

    ignore(isNormal(1.));
    ignore(isNormal(2.));
    ignore(isNormal(3.));
}
