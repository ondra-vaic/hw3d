struct PointLightVectorData
{
    float3 vToL;
    float3 dirToL;
    float distToL;
};

PointLightVectorData CalculateLightVectorData(const in float3 lightPos, const in float3 fragPos)
{
    PointLightVectorData lv;
    lv.vToL = lightPos - fragPos;
    lv.distToL = length(lv.vToL);
    lv.dirToL = lv.vToL / lv.distToL;
    return lv;
}