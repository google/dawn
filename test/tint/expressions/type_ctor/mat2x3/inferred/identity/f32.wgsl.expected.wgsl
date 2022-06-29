var<private> m = mat2x3(mat2x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f));

fn f() -> mat2x3<f32> {
  let m_1 = mat2x3(m);
  return m_1;
}
