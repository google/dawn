alias a = vec3f;

@compute @workgroup_size(1)
fn f() {
  {
    var vec3f = 1;
    var b = vec3f;
  }
  var c = a();
  var d = vec3f();
}
