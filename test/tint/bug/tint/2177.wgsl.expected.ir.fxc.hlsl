SKIP: FAILED


@binding(0) @group(0) var<storage, read_write> arr : array<u32>;

fn f2(p : ptr<storage, array<u32>, read_write>) -> u32 {
  return arrayLength(p);
}

fn f1(p : ptr<storage, array<u32>, read_write>) -> u32 {
  return f2(p);
}

fn f0(p : ptr<storage, array<u32>, read_write>) -> u32 {
  return f1(p);
}

@compute @workgroup_size(1)
fn main() {
  arr[0] = f0(&(arr));
}

Failed to generate: :25:31 error: call: operand is undefined
    %15:u32 = call %f0, %arr, undef
                              ^^^^^

:23:3 note: in block
  $B5: {
  ^^^

note: # Disassembly
$B1: {  # root
  %arr:ptr<storage, array<u32>, read_write> = var @binding_point(0, 0)
}

%f2 = func(%p:ptr<storage, array<u32>, read_write>, %tint_array_length:u32):u32 {
  $B2: {
    ret %tint_array_length
  }
}
%f1 = func(%p_1:ptr<storage, array<u32>, read_write>, %tint_array_length_1:u32):u32 {  # %p_1: 'p', %tint_array_length_1: 'tint_array_length'
  $B3: {
    %8:u32 = call %f2, %p_1, %tint_array_length_1
    ret %8
  }
}
%f0 = func(%p_2:ptr<storage, array<u32>, read_write>, %tint_array_length_2:u32):u32 {  # %p_2: 'p', %tint_array_length_2: 'tint_array_length'
  $B4: {
    %12:u32 = call %f1, %p_2, %tint_array_length_2
    ret %12
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B5: {
    %14:ptr<storage, u32, read_write> = access %arr, 0i
    %15:u32 = call %f0, %arr, undef
    store %14, %15
    ret
  }
}

