struct VertexIn {
    float2 in0: POSITION0;
    float2 in1: POSITION1;
};

struct VertexOut {
    float4 pos: SV_Position;
    float z: POSITION;
};

/*static const float4x4 cMat1 = {
    .866025f, 0.f, .5f, 0.f,//866025
    0.f, 1.f, 0.f, 0.f,
    -.5f,0.f, .866025f, 0.f,//866025
    0.f, 0.f, 0.f, 1.f
};*/

static const float4x4 cMat1 = {
    .0f, 0.f, 1.f, 0.f,
    1.f, 0.f, 0.f, 0.f,
    0.f,-1.f, .0f, 0.f,
    0.f, 0.f, 0.f, 1.f
};

static const float4x4 cMat2 = {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, .5f, .5f,
    0.f, 0.f, 0.f, 1.f
};

VertexOut vertexMain(VertexIn vert, unsigned int vidx: SV_VertexID) {
    VertexOut output;
    //float2 r = float2(dot(vert.in0, vert.in1), vert.in0.y * vert.in1.x - vert.in0.x * vert.in1.y) / (length(vert.in1) + (vidx & 1));
    float2 r = float2(log10(length(vert.in0)) / 5 + 1, 0);
    float4 pos = { r.x, r.y, float(vidx / 2 + 1) / 256.f - 1.f, 1.f };
    output.z = float(vidx/* / 2 + 1*/) / 512.f;
    pos = mul(cMat1, pos);
    output.pos = mul(cMat2, pos);
    return output;
}
