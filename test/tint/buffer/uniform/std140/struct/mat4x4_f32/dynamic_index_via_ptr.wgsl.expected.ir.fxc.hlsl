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
%32 = func(%start_byte_offset:u32):mat4x4<f32> {
  $B4: {
    %54:u32 = div %start_byte_offset, 16u
    %55:ptr<uniform, vec4<u32>, read> = access %a, %54
    %56:vec4<u32> = load %55
    %57:vec4<f32> = bitcast %56
    %58:u32 = add 16u, %start_byte_offset
    %59:u32 = div %58, 16u
    %60:ptr<uniform, vec4<u32>, read> = access %a, %59
    %61:vec4<u32> = load %60
    %62:vec4<f32> = bitcast %61
    %63:u32 = add 32u, %start_byte_offset
    %64:u32 = div %63, 16u
    %65:ptr<uniform, vec4<u32>, read> = access %a, %64
    %66:vec4<u32> = load %65
    %67:vec4<f32> = bitcast %66
    %68:u32 = add 48u, %start_byte_offset
    %69:u32 = div %68, 16u
    %70:ptr<uniform, vec4<u32>, read> = access %a, %69
    %71:vec4<u32> = load %70
    %72:vec4<f32> = bitcast %71
    %73:mat4x4<f32> = construct %57, %62, %67, %72
    ret %73
  }
}
%28 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %75:mat4x4<f32> = call %32, %start_byte_offset_1
    %76:Inner = construct %75
    ret %76
  }
}
%24 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x4<f32>(vec4<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %80:bool = gte %idx, 4u
        if %80 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %81:u32 = mul %idx, 64u
        %82:u32 = add %start_byte_offset_2, %81
        %83:ptr<function, Inner, read_write> = access %a_1, %idx
        %84:Inner = call %28, %82
        store %83, %84
        continue  # -> $B9
      }
      $B9: {  # continuing
        %85:u32 = add %idx, 1u
        next_iteration %85  # -> $B8
      }
    }
    %86:array<Inner, 4> = load %a_1
    ret %86
  }
}
%21 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %88:array<Inner, 4> = call %24, %start_byte_offset_3
    %89:Outer = construct %88
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

