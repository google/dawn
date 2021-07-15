[[stage(compute), workgroup_size(1)]]
fn main() {
  ignore(isNormal(vec4<f32>()));
  ignore(isNormal(vec4<f32>(1.0)));
  ignore(isNormal(vec4<f32>(1.0, 2.0, 3.0, 4.0)));
  ignore(isNormal(vec3<f32>()));
  ignore(isNormal(vec3<f32>(1.0)));
  ignore(isNormal(vec3<f32>(1.0, 2.0, 3.0)));
  ignore(isNormal(vec2<f32>()));
  ignore(isNormal(vec2<f32>(1.0)));
  ignore(isNormal(vec2<f32>(1.0, 2.0)));
  ignore(isNormal(1.0));
  ignore(isNormal(2.0));
  ignore(isNormal(3.0));
}
