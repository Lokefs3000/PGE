Tags={
    RenderMode="Transparent"
    RenderFace="Both"
    Location="Hidden/ImGui"
    Depth="Disabled"
    Cull="None"
}
ShaderSource={
    struct VertexConstantBuffer
    {
        float4x4 ProjectionMatrix; 
    };

    struct VS_INPUT
    {
        float2 pos : POSITION;  
        byte4 col : COLOR0;
        float2 uv  : TEXCOORD0;
    };

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float4 col : COLOR0;
        float2 uv  : TEXCOORD0;
    };

    ConstantBuffer<VertexConstantBuffer> CB : VertexOnly;

    PS_INPUT vertex(VS_INPUT input)
    {
        PS_INPUT output;
        output.pos = mul(CB.ProjectionMatrix, float4(input.pos.xy, 0.0f, 1.0f));
        output.col = input.col;
        output.uv  = input.uv;
        return output;
    }

    StaticSamplerState sampler0 : PixelOnly
    {
        Filter="Linear"
        AddressU="Wrap"
        AddressV="Wrap"
    }

    Texture2D texture0 : PixelOnly;

    float4 pixel(PS_INPUT input) : SV_Target    
    {
        float4 out_col = input.col * texture0.Sample(sampler0, input.uv);
        return out_col;
    }
}