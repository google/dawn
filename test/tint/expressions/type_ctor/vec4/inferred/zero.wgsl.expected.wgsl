var<private> f : vec4f = vec4();

var<private> i : vec4i = vec4();

var<private> u : vec4u = vec4();

@compute @workgroup_size(1)
fn main() {
  _ = f;
  _ = i;
  _ = u;
}
