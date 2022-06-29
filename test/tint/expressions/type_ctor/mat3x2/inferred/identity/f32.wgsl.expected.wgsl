var<private> m = mat3x2(mat3x2(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f));

fn f() -> mat3x2<f32> {
  let m_1 = mat3x2(m);
  return m_1;
}
