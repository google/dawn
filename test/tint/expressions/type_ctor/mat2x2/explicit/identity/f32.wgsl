var<private> m = mat2x2(mat2x2<f32>(0.0f, 1.0f,
                                    2.0f, 3.0f));

fn f() -> mat2x2<f32> {
    let m_1 = mat2x2(m);
    return m_1;
}
