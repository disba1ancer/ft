struct VertexIn {
    float2 pos: POSITION;
};

struct VertexOut {
    float4 pos: SV_Position;
};

VertexOut vertexMain(VertexIn vert, unsigned int vidx: SV_VertexID) {
    VertexOut output;
    output.pos = float4(/*log10(*/vert.pos.x/*) / 2.5f + 1.f*/, vert.pos.y, float(vidx) / 1024.f, 1);
    return output;
}
