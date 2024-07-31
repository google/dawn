SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

var<workgroup> arg_0 : atomic<u32>;

fn atomicSub_0d26c2() -> u32 {
  var arg_1 = 1u;
  var res : u32 = atomicSub(&(arg_0), arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicSub_0d26c2();
}

Failed to generate: :15:5 error: unary: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'

    %7:u32 = negation %5
    ^^^^^^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
compute_main_inputs = struct @align(4) {
  tint_local_index:u32 @offset(0), @builtin(local_invocation_index)
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
  %arg_0:ptr<workgroup, atomic<u32>, read_write> = var
}

%atomicSub_0d26c2 = func():u32 {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var, 1u
    %5:u32 = load %arg_1
    %6:ptr<function, u32, read_write> = var, 0u
    %7:u32 = negation %5
    %8:void = hlsl.InterlockedAdd %arg_0, %7, %6
    %9:u32 = load %6
    %res:ptr<function, u32, read_write> = var, %9
    %11:u32 = load %res
    ret %11
  }
}
%compute_main_inner = func(%tint_local_index:u32):void {
  $B3: {
    %14:bool = eq %tint_local_index, 0u
    if %14 [t: $B4] {  # if_1
      $B4: {  # true
        %15:ptr<function, u32, read_write> = var, 0u
        %16:void = hlsl.InterlockedExchange %arg_0, 0u, %15
        exit_if  # if_1
      }
    }
    %17:void = workgroupBarrier
    %18:u32 = call %atomicSub_0d26c2
    %19:void = %prevent_dce.Store 0u, %18
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func(%inputs:compute_main_inputs):void {
  $B5: {
    %22:u32 = access %inputs, 0u
    %23:void = call %compute_main_inner, %22
    ret
  }
}

