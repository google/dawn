intrinsics/repeated_use.wgsl:5:9 warning: use of deprecated intrinsic
    _ = isNormal(vec4<f32>());
        ^^^^^^^^

intrinsics/repeated_use.wgsl:6:9 warning: use of deprecated intrinsic
    _ = isNormal(vec4<f32>(1.));
        ^^^^^^^^

intrinsics/repeated_use.wgsl:7:9 warning: use of deprecated intrinsic
    _ = isNormal(vec4<f32>(1., 2., 3., 4.));
        ^^^^^^^^

intrinsics/repeated_use.wgsl:9:9 warning: use of deprecated intrinsic
    _ = isNormal(vec3<f32>());
        ^^^^^^^^

intrinsics/repeated_use.wgsl:10:9 warning: use of deprecated intrinsic
    _ = isNormal(vec3<f32>(1.));
        ^^^^^^^^

intrinsics/repeated_use.wgsl:11:9 warning: use of deprecated intrinsic
    _ = isNormal(vec3<f32>(1., 2., 3.));
        ^^^^^^^^

intrinsics/repeated_use.wgsl:13:9 warning: use of deprecated intrinsic
    _ = isNormal(vec2<f32>());
        ^^^^^^^^

intrinsics/repeated_use.wgsl:14:9 warning: use of deprecated intrinsic
    _ = isNormal(vec2<f32>(1.));
        ^^^^^^^^

intrinsics/repeated_use.wgsl:15:9 warning: use of deprecated intrinsic
    _ = isNormal(vec2<f32>(1., 2.));
        ^^^^^^^^

intrinsics/repeated_use.wgsl:17:9 warning: use of deprecated intrinsic
    _ = isNormal(1.);
        ^^^^^^^^

intrinsics/repeated_use.wgsl:18:9 warning: use of deprecated intrinsic
    _ = isNormal(2.);
        ^^^^^^^^

intrinsics/repeated_use.wgsl:19:9 warning: use of deprecated intrinsic
    _ = isNormal(3.);
        ^^^^^^^^

[[stage(compute), workgroup_size(1)]]
fn main() {
  _ = isNormal(vec4<f32>());
  _ = isNormal(vec4<f32>(1.0));
  _ = isNormal(vec4<f32>(1.0, 2.0, 3.0, 4.0));
  _ = isNormal(vec3<f32>());
  _ = isNormal(vec3<f32>(1.0));
  _ = isNormal(vec3<f32>(1.0, 2.0, 3.0));
  _ = isNormal(vec2<f32>());
  _ = isNormal(vec2<f32>(1.0));
  _ = isNormal(vec2<f32>(1.0, 2.0));
  _ = isNormal(1.0);
  _ = isNormal(2.0);
  _ = isNormal(3.0);
}
