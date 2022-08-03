enable f16;

var<private> m = mat4x2(mat4x2(0.0h, 1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h, 7.0h));

fn f() -> mat4x2<f16> {
  let m_1 = mat4x2(m);
  return m_1;
}
