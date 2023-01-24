alias ArrayType = array<i32, 4>;

struct S {
  arr : array<i32, 4>,
};

fn foo() {
  let src : ArrayType = ArrayType();

  var dst : ArrayType;
  var dst_struct : S;
  var dst_array : array<ArrayType, 2>;
  let dst_ptr : ptr<function, ArrayType> = &dst;
  let dst_struct_ptr : ptr<function, S> = &dst_struct;
  let dst_array_ptr : ptr<function, array<ArrayType, 2>> = &dst_array;

  // Assign to struct.member.
  dst_struct.arr = src;

  // Assign to array[index].
  dst_array[1] = src;

  // Assign via pointers.
  *dst_ptr = src;
  (*dst_struct_ptr).arr = src;
  (*dst_array_ptr)[0] = src;
}
