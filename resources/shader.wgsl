struct VertexInput {
    @location(0) position: vec2f,
    @location(1) color: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    // location here does _NOT_ refer to a vertex attribute 
    // it means that this field must be handled by the rasterizer
    // it could also refer to another field of another struct that would be used as input to the fragment shader
    @location(0) color: vec3f,
};

/*
 * the `@location(0)` attribute means this input variable is described by the vertex buffer layout at index 0 in the `pipelineDesc.vertex.buffers` array 
* type `vec2f` must comply with what is declared in the layout  
*/

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position, 0.0, 1.0);
    out.color = in.color;
    let ratio = 640.0 / 480.0;
    // out.position = vec4f(in.position.x, in.position.y * ratio, 0.0, 1.0);
    let offset = vec2f(-0.6875, -0.463);
    out.position = vec4f(in.position.x + offset.x, (in.position.y + offset.y) * ratio, 0.0, 1.0);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    // apply a gamma-correction to the color
    // convert the input sRGB color into linear before the target surface converts it back to sRGB
    // because by default WebGPU assumes the colors output by fragment shader are linear
    let linear_color = pow(in.color, vec3f(2.2));
    return vec4f(linear_color, 1.0);
    // return vec4f(in.color, 1.0);
}
