enable f16;

var<private> m = mat2x3(mat2x3<f16>(0.0h, 1.0h, 2.0h, 3.0h, 4.0h, 5.0h));

fn f() -> mat2x3<f16> {
  let m_1 = mat2x3(m);
  return m_1;
}
