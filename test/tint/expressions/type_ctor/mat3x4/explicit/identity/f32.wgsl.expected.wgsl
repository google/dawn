var<private> m = mat3x4(mat3x4<f32>(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f));

fn f() -> mat3x4<f32> {
  let m_1 = mat3x4(m);
  return m_1;
}
