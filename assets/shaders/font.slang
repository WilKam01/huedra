struct VSInput
{
    float2 position;
    float2 size;
    uint32_t startContour;
    uint32_t endContour;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : UV;
    uint32_t startContour : START_CONTOUR;
    uint32_t endContour : END_CONTOUR;
};

struct TextInfo
{
    float2 origin;
    float2 size;
    float4x4 projection;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, ConstantBuffer<TextInfo> info, uint vertexID: SV_VertexID)
{
    VSOutput output;
    float2 position = float2(vertexID & 1, (vertexID >> 1) & 1);
    output.position =
        mul(info.projection, float4(info.origin + (input.position + position * input.size) * info.size, 0.0f, 1.0f));
    output.uv = position;
    output.startContour = input.startContour;
    output.endContour = input.endContour;
    return output;
}

struct Range
{
    uint32_t start;
    uint32_t end;
};

struct FontData
{
    StructuredBuffer<float2> points;
    StructuredBuffer<Range> contourIndexRanges; // Range of points used in a contour
};

struct FSInput
{
    float2 uv : UV;
    uint32_t startContour : START_CONTOUR;
    uint32_t endContour : END_CONTOUR;
};

struct FSOutput
{
    float4 outColor : SV_Target;
};

float2 bezier(float2 p0, float2 p1, float2 p2, float t)
{
    float u = 1.0f - t;
    return u * u * p0 + 2.0f * u * t * p1 + t * t * p2;
}

float2 bezierTangent(float2 p0, float2 p1, float2 p2, float t)
{
    return normalize(2.0f * (1.0f - t) * (p1 - p0) + 2.0f * t * (p2 - p1));
}

bool checkHeightRange(float2 point, float2 p0, float2 p1, float2 p2)
{
    float t = (p0.y - p1.y) / (p0.y - 2.0f * p1.y + p2.y);
    float2 b = bezier(p0, p1, p2, t);
    float minY = min(p1.y, min(p0.y, p2.y));
    float maxY = max(p1.y, max(p0.y, p2.y));
    if (t >= -1e-6f && t <= 1.0f + 1e-6f)
    {
        minY = min(minY, b.y);
        maxY = max(maxY, b.y);
    }
    return point.y >= minY && point.y <= maxY;
}

struct HorizontalDistanceInfo
{
    float leftDist;
    float rightDist;
    float2 leftTangent;
    float2 rightTangent;
}

HorizontalDistanceInfo getHorizontalDistance(float2 point, float2 p0, float2 p1, float2 p2)
{
    HorizontalDistanceInfo info;
    info.leftDist = -1e6f;
    info.rightDist = 1e6f;
    info.leftTangent = float2(0.0f);
    info.rightTangent = float2(0.0f);

    // Solve for t where y(t) = point.y
    float ay = p0.y - 2.0f * p1.y + p2.y;
    float by = -2.0f * p0.y + 2.0f * p1.y;
    float cy = p0.y - point.y;

    // Linear case
    if (abs(ay) < 1e-6f && abs(by) > 1e-6f)
    {
        float t = -cy / by;
        if (t >= -1e-6f && t <= 1.0f + 1e-6f)
        {
            float2 b = bezier(p0, p1, p2, t);
            float dist = b.x - point.x;
            if (dist < 0.0f)
            {
                info.leftDist = dist;
                info.leftTangent = bezierTangent(p0, p1, p2, t);
            }
            else if (dist > 0.0f)
            {
                info.rightDist = dist;
                info.rightTangent = bezierTangent(p0, p1, p2, t);
            }
        }
    }
    // Quadratic case
    else
    {
        float discriminant = by * by - 4.0f * ay * cy;
        if (discriminant >= 1e-6f)
        {
            float sqrtD = sqrt(discriminant);
            float t0 = (-by + sqrtD) / (2.0f * ay);
            float t1 = (-by - sqrtD) / (2.0f * ay);
            if (t0 >= -1e-6f && t0 <= 1.0f + 1e-6f)
            {
                float2 b = bezier(p0, p1, p2, t0);
                float dist = b.x - point.x;
                if (dist < 0.0f)
                {
                    info.leftDist = dist;
                    info.leftTangent = bezierTangent(p0, p1, p2, t0);
                }
                else if (dist > 0.0f)
                {
                    info.rightDist = dist;
                    info.rightTangent = bezierTangent(p0, p1, p2, t0);
                }
            }
            if (t1 >= -1e-6f && t1 <= 1.0f + 1e-6f)
            {
                float2 b = bezier(p0, p1, p2, t1);
                float dist = b.x - point.x;
                if (dist < 0.0f && dist > info.leftDist)
                {
                    info.leftDist = dist;
                    info.leftTangent = bezierTangent(p0, p1, p2, t1);
                }
                else if (dist > 0.0f && dist < info.rightDist)
                {
                    info.rightDist = dist;
                    info.rightTangent = bezierTangent(p0, p1, p2, t1);
                }
            }
        }
    }

    return info;
}

[shader("fragment")]
FSOutput fragMain(FSInput input, ParameterBlock<FontData> fontData)
{
    FSOutput output;
    bool valid = false;

    float closestLeftContourDist = -1e6;
    bool closestLeftContourYDir = false;
    bool closestLeftContourIsFilling = false;

    float closestRightContourDist = 1e6;
    bool closestRightContourYDir = false;
    bool closestRightContourIsFilling = false;

    for (int i = input.startContour; i <= input.endContour; ++i)
    {
        Range contour = fontData.contourIndexRanges[i];
        float area = 0.0f;

        float leftDist = -1e6;
        bool leftYDir = false;
        bool leftValid = false;

        float rightDist = 1e6;
        bool rightYDir = false;
        bool rightValid = false;

        int numIterations = contour.end - contour.start;
        for (int j = 0; j <= numIterations; j += 2)
        {
            float2 p0 = fontData.points[j + contour.start];
            float2 p1 = fontData.points[j + 1 + contour.start];
            float2 p2 = fontData.points[(j + 2) % (numIterations + 1) + contour.start];
            if (checkHeightRange(input.uv, p0, p1, p2))
            {
                HorizontalDistanceInfo info = getHorizontalDistance(input.uv, p0, p1, p2);
                if (info.leftDist < 0.0f && info.leftDist > leftDist)
                {
                    leftDist = info.leftDist;
                    leftYDir = info.leftTangent.y > 0.0f;
                    leftValid = true;
                }
                else if (info.rightDist > 0.0f && info.rightDist < rightDist)
                {
                    rightDist = info.rightDist;
                    rightYDir = info.rightTangent.y > 0.0f;
                    rightValid = true;
                }
            }

            area += p0.x * p2.y - p2.x * p0.y;
        }

        bool goingClockWise = area < -1e-6f;

        if (leftValid && rightValid && (leftYDir || !rightYDir))
        {
            valid = true;
        }

        if (leftValid && leftDist > closestLeftContourDist)
        {
            closestLeftContourDist = leftDist;
            closestLeftContourYDir = leftYDir;
            closestLeftContourIsFilling = goingClockWise;
        }
        if (rightValid && rightDist < closestRightContourDist)
        {
            closestRightContourDist = rightDist;
            closestRightContourYDir = rightYDir;
            closestRightContourIsFilling = goingClockWise;
        }
    }

    if (!valid || !(closestLeftContourYDir || (closestLeftContourIsFilling && closestRightContourIsFilling) ||
                    (!closestRightContourYDir && (closestLeftContourIsFilling || closestRightContourIsFilling))))
    {
        discard;
    }

    output.outColor = float4(1.0f);
    return output;
}
