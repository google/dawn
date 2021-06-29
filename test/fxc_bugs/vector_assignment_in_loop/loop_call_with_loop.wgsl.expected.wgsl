var<private> v2f : vec2<f32>;

var<private> v3i : vec3<i32>;

var<private> v4u : vec4<u32>;

var<private> v2b : vec2<bool>;

fn foo() {
  {
    var i : i32 = 0;
    loop {
      if (!((i < 2))) {
        break;
      }
      v2f[i] = 1.0;
      v3i[i] = 1;
      v4u[i] = 1u;
      v2b[i] = true;

      continuing {
        i = (i + 1);
      }
    }
  }
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  {
    var i : i32 = 0;
    loop {
      if (!((i < 2))) {
        break;
      }
      foo();

      continuing {
        i = (i + 1);
      }
    }
  }
}
