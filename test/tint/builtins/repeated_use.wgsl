// Check that for backends that generate builtin helpers, repeated use of the
// same builtin overload results in single helper being generated.
@compute @workgroup_size(1)
fn main() {
    let a = degrees(vec4<f32>());
    let b = degrees(vec4<f32>(1.));
    let c = degrees(vec4<f32>(1., 2., 3., 4.));

    let d = degrees(vec3<f32>());
    let e = degrees(vec3<f32>(1.));
    let f = degrees(vec3<f32>(1., 2., 3.));

    let g = degrees(vec2<f32>());
    let h = degrees(vec2<f32>(1.));
    let i = degrees(vec2<f32>(1., 2.));

    let j = degrees(1.);
    let k = degrees(2.);
    let l = degrees(3.);
}
