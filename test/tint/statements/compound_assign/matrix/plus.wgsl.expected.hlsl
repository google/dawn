SKIP: FAILED


struct S {
  a : mat4x4<f32>,
}

@group(0) @binding(0) var<storage, read_write> v : S;

fn foo() {
  v.a += mat4x4<f32>();
}

Failed to generate: error: cannot assign to value of type 'mat4x4<f32>'
