var<private> f : vec2f = vec2();

var<private> i : vec2i = vec2();

var<private> u : vec2u = vec2();

@compute @workgroup_size(1)
fn main() {
  _ = f;
  _ = i;
  _ = u;
}
