enable f16;

var<private> m = mat3x3(mat3x3<f16>(0.0h, 1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h, 7.0h, 8.0h));

fn f() -> mat3x3<f16> {
  let m_1 = mat3x3(m);
  return m_1;
}
