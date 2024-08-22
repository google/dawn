SKIP: FAILED


struct Inner {
  @size(64)
  m : mat4x4<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat4x4<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec4<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :54:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %42:u32 = mul %41, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(16) {
  m:mat4x4<f32> @offset(0)
}

Outer = struct @align(16) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 16u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:array<Inner, 4> = call %24, %10
    %l_a_i_a:array<Inner, 4> = let %23
    %26:u32 = add %10, %13
    %27:Inner = call %28, %26
    %l_a_i_a_i:Inner = let %27
    %30:u32 = add %10, %13
    %31:mat4x4<f32> = call %32, %30
    %l_a_i_a_i_m:mat4x4<f32> = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = div %35, 16u
    %37:ptr<uniform, vec4<u32>, read> = access %a, %36
    %38:vec4<u32> = load %37
    %39:vec4<f32> = bitcast %38
    %l_a_i_a_i_m_i:vec4<f32> = let %39
    %41:i32 = call %i
    %42:u32 = mul %41, 4u
    %43:u32 = add %10, %13
    %44:u32 = add %43, %16
    %45:u32 = add %44, %42
    %46:u32 = div %45, 16u
    %47:ptr<uniform, vec4<u32>, read> = access %a, %46
    %48:u32 = mod %45, 16u
    %49:u32 = div %48, 4u
    %50:u32 = load_vector_element %47, %49
    %51:f32 = bitcast %50
    %l_a_i_a_i_m_i_i:f32 = let %51
    ret
  }
}
%21 = func(%start_byte_offset:u32):Outer {
  $B4: {
    %54:array<Inner, 4> = call %24, %start_byte_offset
    %55:Outer = construct %54
    ret %55
  }
}
%24 = func(%start_byte_offset_1:u32):array<Inner, 4> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x4<f32>(vec4<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B6, b: $B7, c: $B8] {  # loop_1
      $B6: {  # initializer
        next_iteration 0u  # -> $B7
      }
      $B7 (%idx:u32): {  # body
        %59:bool = gte %idx, 4u
        if %59 [t: $B9] {  # if_1
          $B9: {  # true
            exit_loop  # loop_1
          }
        }
        %60:u32 = mul %idx, 64u
        %61:u32 = add %start_byte_offset_1, %60
        %62:ptr<function, Inner, read_write> = access %a_1, %idx
        %63:Inner = call %28, %61
        store %62, %63
        continue  # -> $B8
      }
      $B8: {  # continuing
        %64:u32 = add %idx, 1u
        next_iteration %64  # -> $B7
      }
    }
    %65:array<Inner, 4> = load %a_1
    ret %65
  }
}
%28 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %67:mat4x4<f32> = call %32, %start_byte_offset_2
    %68:Inner = construct %67
    ret %68
  }
}
%32 = func(%start_byte_offset_3:u32):mat4x4<f32> {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %70:u32 = div %start_byte_offset_3, 16u
    %71:ptr<uniform, vec4<u32>, read> = access %a, %70
    %72:vec4<u32> = load %71
    %73:vec4<f32> = bitcast %72
    %74:u32 = add 16u, %start_byte_offset_3
    %75:u32 = div %74, 16u
    %76:ptr<uniform, vec4<u32>, read> = access %a, %75
    %77:vec4<u32> = load %76
    %78:vec4<f32> = bitcast %77
    %79:u32 = add 32u, %start_byte_offset_3
    %80:u32 = div %79, 16u
    %81:ptr<uniform, vec4<u32>, read> = access %a, %80
    %82:vec4<u32> = load %81
    %83:vec4<f32> = bitcast %82
    %84:u32 = add 48u, %start_byte_offset_3
    %85:u32 = div %84, 16u
    %86:ptr<uniform, vec4<u32>, read> = access %a, %85
    %87:vec4<u32> = load %86
    %88:vec4<f32> = bitcast %87
    %89:mat4x4<f32> = construct %73, %78, %83, %88
    ret %89
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat4x4<f32>(vec4<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %93:bool = gte %idx_1, 4u
        if %93 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %94:u32 = mul %idx_1, 256u
        %95:u32 = add %start_byte_offset_4, %94
        %96:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %97:Outer = call %21, %95
        store %96, %97
        continue  # -> $B15
      }
      $B15: {  # continuing
        %98:u32 = add %idx_1, 1u
        next_iteration %98  # -> $B14
      }
    }
    %99:array<Outer, 4> = load %a_2
    ret %99
  }
}

