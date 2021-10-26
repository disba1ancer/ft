struct VertexOut {
    float4 pos: SV_Position;
};

float4 fragmentMain() : SV_Target {
    return float4(1, 1, 0, 1);
}
