#!/usr/bin/env python
# coding: utf-8

# In[1]:


import dawn
import glfw
import glm
import numpy as np

# Initialize GLFW
if not glfw.init():
    raise RuntimeError("Failed to initialize GLFW")

# Create a window
window = glfw.create_window(800, 600, "Dawn 3D Scene Renderer", None, None)
if not window:
    glfw.terminate()
    raise RuntimeError("Failed to create window")

glfw.make_context_current(window)

# Initialize Dawn
instance = dawn.Instance()
instance.discover_default_adapters()
adapter = instance.get_adapter(dawn.BackendType.D3D12)
device = adapter.create_device()

# Define vertex and fragment shaders
vertex_shader_code = """
    [[stage(vertex)]]
    fn main([[location(0)]] position: vec3<f32>) -> [[builtin(position)]] vec4<f32> {
        return vec4<f32>(position, 1.0);
    }
"""

fragment_shader_code = """
    [[stage(fragment)]]
    fn main() -> [[location(0)]] vec4<f32> {
        return vec4<f32>(1.0, 0.0, 0.0, 1.0); // Red color
    }
"""

# Create shader modules
vs_module_desc = dawn.ShaderModuleDescriptor(dawn.ShaderStage.VERTEX, vertex_shader_code)
fs_module_desc = dawn.ShaderModuleDescriptor(dawn.ShaderStage.FRAGMENT, fragment_shader_code)
vs_module = device.create_shader_module(vs_module_desc)
fs_module = device.create_shader_module(fs_module_desc)

# Define a triangle
vertices = np.array([
    [-0.5, -0.5, 0.0],
    [0.5, -0.5, 0.0],
    [0.0, 0.5, 0.0]
], dtype=np.float32)

vertex_buffer = device.create_buffer_mapped(dawn.BufferUsage.VERTEX, size=len(vertices) * 4)
vertex_buffer.set_data(vertices)

# Main loop
while not glfw.window_should_close(window):
    glfw.poll_events()

    # Acquire the next texture from the swap chain
    surface = dawn.Surface()
    swap_chain_desc = dawn.SwapChainDescriptor(usage=dawn.TextureUsage.RENDER_ATTACHMENT)
    swap_chain = device.create_swap_chain(surface, swap_chain_desc)
    texture_view = swap_chain.get_current_texture_view()

    # Create a command encoder
    encoder = device.create_command_encoder()
    render_pass_desc = dawn.RenderPassDescriptor(
        color_attachments=[dawn.RenderPassColorAttachment(
            view=texture_view,
            load_op=dawn.LoadOp.CLEAR,
            store_op=dawn.StoreOp.STORE,
            clear_color=dawn.Color(0.0, 0.0, 0.0, 1.0)
        )]
    )
    pass_encoder = encoder.begin_render_pass(render_pass_desc)

    # Set pipeline and bind vertex buffer
    pipeline_layout_desc = dawn.PipelineLayoutDescriptor()
    pipeline_desc = dawn.RenderPipelineDescriptor(
        layout=device.create_pipeline_layout(pipeline_layout_desc),
        vertex_stage=dawn.ProgrammableStageDescriptor(vs_module, "main"),
        fragment_stage=dawn.ProgrammableStageDescriptor(fs_module, "main"),
        primitive_topology=dawn.PrimitiveTopology.TRIANGLE_LIST,
        color_states=[dawn.ColorStateDescriptor(format=dawn.TextureFormat.RGBA8_UNORM)],
    )
    pipeline = device.create_render_pipeline(pipeline_desc)
    pass_encoder.set_pipeline(pipeline)
    pass_encoder.set_vertex_buffer(0, vertex_buffer)

    # Draw the triangle
    pass_encoder.draw(3)

    # End render pass and command encoder
    pass_encoder.end_pass()
    cmd_buffer = encoder.finish()

    # Submit command buffer to the queue
    queue = device.get_queue()
    queue.submit([cmd_buffer])

    # Present the rendered frame
    swap_chain.present()

# Clean up
glfw.terminate()


# In[ ]:




