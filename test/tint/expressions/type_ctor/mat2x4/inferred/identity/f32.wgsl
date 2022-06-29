var<private> m = mat2x4(mat2x4(0.0f, 1.0f, 2.0f, 3.0f,
                               4.0f, 5.0f, 6.0f, 7.0f));

fn f() -> mat2x4<f32> {
    let m_1 = mat2x4(m);
    return m_1;
}
