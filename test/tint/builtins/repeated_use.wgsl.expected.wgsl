builtins/repeated_use.wgsl:5:9 warning: use of deprecated builtin
    _ = isNormal(vec4<f32>());
        ^^^^^^^^

builtins/repeated_use.wgsl:6:9 warning: use of deprecated builtin
    _ = isNormal(vec4<f32>(1.));
        ^^^^^^^^

builtins/repeated_use.wgsl:7:9 warning: use of deprecated builtin
    _ = isNormal(vec4<f32>(1., 2., 3., 4.));
        ^^^^^^^^

builtins/repeated_use.wgsl:9:9 warning: use of deprecated builtin
    _ = isNormal(vec3<f32>());
        ^^^^^^^^

builtins/repeated_use.wgsl:10:9 warning: use of deprecated builtin
    _ = isNormal(vec3<f32>(1.));
        ^^^^^^^^

builtins/repeated_use.wgsl:11:9 warning: use of deprecated builtin
    _ = isNormal(vec3<f32>(1., 2., 3.));
        ^^^^^^^^

builtins/repeated_use.wgsl:13:9 warning: use of deprecated builtin
    _ = isNormal(vec2<f32>());
        ^^^^^^^^

builtins/repeated_use.wgsl:14:9 warning: use of deprecated builtin
    _ = isNormal(vec2<f32>(1.));
        ^^^^^^^^

builtins/repeated_use.wgsl:15:9 warning: use of deprecated builtin
    _ = isNormal(vec2<f32>(1., 2.));
        ^^^^^^^^

builtins/repeated_use.wgsl:17:9 warning: use of deprecated builtin
    _ = isNormal(1.);
        ^^^^^^^^

builtins/repeated_use.wgsl:18:9 warning: use of deprecated builtin
    _ = isNormal(2.);
        ^^^^^^^^

builtins/repeated_use.wgsl:19:9 warning: use of deprecated builtin
    _ = isNormal(3.);
        ^^^^^^^^

@stage(compute) @workgroup_size(1)
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
