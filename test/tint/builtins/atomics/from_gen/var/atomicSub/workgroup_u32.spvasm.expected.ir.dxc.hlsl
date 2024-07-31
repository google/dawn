SKIP: FAILED


var<private> local_invocation_index_1 : u32;

var<workgroup> arg_0 : atomic<u32>;

fn atomicSub_0d26c2() {
  var arg_1 = 0u;
  var res = 0u;
  arg_1 = 1u;
  let x_18 = arg_1;
  let x_14 = atomicSub(&(arg_0), x_18);
  res = x_14;
  return;
}

fn compute_main_inner(local_invocation_index_2 : u32) {
  atomicStore(&(arg_0), 0u);
  workgroupBarrier();
  atomicSub_0d26c2();
  return;
}

fn compute_main_1() {
  let x_32 = local_invocation_index_1;
  compute_main_inner(x_32);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

Failed to generate: :18:5 error: unary: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'

    %9:u32 = negation %x_18
    ^^^^^^^^^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
compute_main_inputs = struct @align(4) {
  local_invocation_index_1_param:u32 @offset(0), @builtin(local_invocation_index)
}

$B1: {  # root
  %local_invocation_index_1:ptr<private, u32, read_write> = var
  %arg_0:ptr<workgroup, atomic<u32>, read_write> = var
}

%atomicSub_0d26c2 = func():void {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var, 0u
    %res:ptr<function, u32, read_write> = var, 0u
    store %arg_1, 1u
    %6:u32 = load %arg_1
    %x_18:u32 = let %6
    %8:ptr<function, u32, read_write> = var, 0u
    %9:u32 = negation %x_18
    %10:void = hlsl.InterlockedAdd %arg_0, %9, %8
    %11:u32 = load %8
    %x_14:u32 = let %11
    store %res, %x_14
    ret
  }
}
%compute_main_inner = func(%local_invocation_index_2:u32):void {
  $B3: {
    %15:ptr<function, u32, read_write> = var, 0u
    %16:void = hlsl.InterlockedExchange %arg_0, 0u, %15
    %17:void = workgroupBarrier
    %18:void = call %atomicSub_0d26c2
    ret
  }
}
%compute_main_1 = func():void {
  $B4: {
    %20:u32 = load %local_invocation_index_1
    %x_32:u32 = let %20
    %22:void = call %compute_main_inner, %x_32
    ret
  }
}
%compute_main_inner_1 = func(%local_invocation_index_1_param:u32):void {  # %compute_main_inner_1: 'compute_main_inner'
  $B5: {
    %25:bool = eq %local_invocation_index_1_param, 0u
    if %25 [t: $B6] {  # if_1
      $B6: {  # true
        %26:ptr<function, u32, read_write> = var, 0u
        %27:void = hlsl.InterlockedExchange %arg_0, 0u, %26
        exit_if  # if_1
      }
    }
    %28:void = workgroupBarrier
    store %local_invocation_index_1, %local_invocation_index_1_param
    %29:void = call %compute_main_1
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func(%inputs:compute_main_inputs):void {
  $B7: {
    %32:u32 = access %inputs, 0u
    %33:void = call %compute_main_inner_1, %32
    ret
  }
}

