fn bitwise_i32() {
  var s1 : i32;
  var s2 : i32;
  var v1 : vec3<i32>;
  var v2 : vec3<i32>;
  s1 = (s1 | s2);
  s1 = (s1 & s2);
  s1 = (s1 ^ s2);
  v1 = (v1 | v2);
  v1 = (v1 & v2);
  v1 = (v1 ^ v2);
}

fn bitwise_u32() {
  var s1 : u32;
  var s2 : u32;
  var v1 : vec3<u32>;
  var v2 : vec3<u32>;
  s1 = (s1 | s2);
  s1 = (s1 & s2);
  s1 = (s1 ^ s2);
  v1 = (v1 | v2);
  v1 = (v1 & v2);
  v1 = (v1 ^ v2);
}

fn vector_scalar_f32() {
  var v : vec3<f32>;
  var s : f32;
  var r : vec3<f32>;
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
}

fn vector_scalar_i32() {
  var v : vec3<i32>;
  var s : i32;
  var r : vec3<i32>;
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
  r = (v % s);
}

fn vector_scalar_u32() {
  var v : vec3<u32>;
  var s : u32;
  var r : vec3<u32>;
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
  r = (v % s);
}

fn scalar_vector_f32() {
  var v : vec3<f32>;
  var s : f32;
  var r : vec3<f32>;
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
}

fn scalar_vector_i32() {
  var v : vec3<i32>;
  var s : i32;
  var r : vec3<i32>;
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
  r = (s % v);
}

fn scalar_vector_u32() {
  var v : vec3<u32>;
  var s : u32;
  var r : vec3<u32>;
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
  r = (s % v);
}

fn matrix_matrix_f32() {
  var m34 : mat3x4<f32>;
  var m43 : mat4x3<f32>;
  var m33 : mat3x3<f32>;
  var m44 : mat4x4<f32>;
  m34 = (m34 + m34);
  m34 = (m34 - m34);
  m33 = (m43 * m34);
  m44 = (m34 * m43);
}

[[stage(fragment)]]
fn main() -> [[location(0)]] vec4<f32> {
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}
