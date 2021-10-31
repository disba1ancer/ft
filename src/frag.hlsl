struct VertexOut {
    float4 pos: SV_Position;
    float z: POSITION;
};

float color_helper(float val) {
    return clamp(-abs(6.f * val - 2.f) + 2.f, 0.f, 1.f);
}

float color_helper_red(float val) {
    return color_helper(frac(val + 0.333333f));
}

float color_helper_green(float val) {
    return color_helper(frac(val));
}

float color_helper_blue(float val) {
    return color_helper(frac(val - .333333f));
}

float4 get_color(float val) {
    return float4(color_helper_red(val), color_helper_green(val), color_helper_blue(val), 1.f);
}

float4 fragmentMain(VertexOut input) : SV_Target {
    return get_color(input.z);
}
