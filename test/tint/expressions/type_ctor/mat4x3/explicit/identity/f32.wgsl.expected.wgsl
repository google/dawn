var<private> m = mat4x3(mat4x3<f32>(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f));

fn f() -> mat4x3<f32> {
  let m_1 = mat4x3(m);
  return m_1;
}
