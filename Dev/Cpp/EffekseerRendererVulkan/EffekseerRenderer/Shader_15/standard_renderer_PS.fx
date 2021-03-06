#version 420

struct PS_Input
{
    vec4 Pos;
    vec4 Color;
    vec2 UV;
};

layout(set = 1, binding = 1) uniform sampler2D Sampler_g_sampler;

layout(location = 0) in vec4 Input_Color;
layout(location = 1) in vec2 Input_UV;
layout(location = 0) out vec4 _entryPointOutput;

vec4 _main(PS_Input Input)
{
    vec4 Output = Input.Color * texture(Sampler_g_sampler, Input.UV);
    if (Output.w == 0.0)
    {
        discard;
    }
    return Output;
}

void main()
{
    PS_Input Input;
    Input.Pos = gl_FragCoord;
    Input.Color = Input_Color;
    Input.UV = Input_UV;
    vec4 _65 = _main(Input);
    _entryPointOutput = _65;
}

