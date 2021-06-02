type ArrayType = [[stride(16)]] array<i32, 4>;

[[block]]
struct S {
  arr : ArrayType;
};

[[block]]
struct S_nested {
  arr : array<array<array<i32, 2>, 3>, 4>;
};

var<private> src_private : ArrayType;
var<workgroup> src_workgroup : ArrayType;
[[group(0), binding(0)]] var<uniform> src_uniform : S;
[[group(0), binding(1)]] var<storage> src_storage : [[access(read_write)]] S;

[[group(0), binding(2)]] var<storage> dst : [[access(read_write)]] S;
[[group(0), binding(3)]] var<storage> dst_nested : [[access(read_write)]] S_nested;

fn ret_arr() -> ArrayType {
  return ArrayType();
}

fn ret_struct_arr() -> S {
  return S();
}

fn foo(src_param : ArrayType) {
  var src_function : ArrayType;

  // Assign from type constructor.
  dst.arr = ArrayType(1, 2, 3, 3);

  // Assign from parameter.
  dst.arr = src_param;

  // Assign from function call.
  dst.arr = ret_arr();

  // Assign from constant.
  let src_let : ArrayType = ArrayType();
  dst.arr = src_let;

  // Assign from var, various storage classes.
  dst.arr = src_function;
  dst.arr = src_private;
  dst.arr = src_workgroup;

  // Assign from struct.arr, various storage classes.
  dst.arr = ret_struct_arr().arr;
  dst.arr = src_uniform.arr;
  dst.arr = src_storage.arr;

  // Nested assignment.
  var src_nested : array<array<array<i32, 2>, 3>, 4>;
  dst_nested.arr = src_nested;
}
