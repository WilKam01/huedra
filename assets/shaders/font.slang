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
    float2 pixelSize : PIXEL_SIZE;
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
    output.pixelSize = float2(1.0f) / (input.size * info.size * 2.0f);
    output.uv = position - (output.pixelSize * position);
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
    float2 pixelSize : PIXEL_SIZE;
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
};

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

struct ClosestInfo
{
    float dist;
    bool yDir; // true = Up, false = Down
    bool valid;
};

[shader("fragment")]
FSOutput fragMain(FSInput input, ParameterBlock<FontData> fontData)
{
    FSOutput output;

    ClosestInfo closestBottomLeft;
    closestBottomLeft.dist = -1e6;
    closestBottomLeft.yDir = false;
    closestBottomLeft.valid = false;

    ClosestInfo closestBottomRight;
    closestBottomRight.dist = 1e6;
    closestBottomRight.yDir = false;
    closestBottomRight.valid = false;

    ClosestInfo closestTopLeft;
    closestTopLeft.dist = -1e6;
    closestTopLeft.yDir = false;
    closestTopLeft.valid = false;

    ClosestInfo closestTopRight;
    closestTopRight.dist = 1e6;
    closestTopRight.yDir = false;
    closestTopRight.valid = false;

    for (int i = input.startContour; i <= input.endContour; ++i)
    {
        Range contour = fontData.contourIndexRanges[i];
        float area = 0.0f;

        ClosestInfo bottomLeft;
        bottomLeft.dist = -1e6;
        bottomLeft.yDir = false;
        bottomLeft.valid = false;

        ClosestInfo topLeft;
        topLeft.dist = -1e6;
        topLeft.yDir = false;
        topLeft.valid = false;

        ClosestInfo bottomRight;
        bottomRight.dist = 1e6;
        bottomRight.yDir = false;
        bottomRight.valid = false;

        ClosestInfo topRight;
        topRight.dist = 1e6;
        topRight.yDir = false;
        topRight.valid = false;

        int numIterations = contour.end - contour.start;
        for (int j = 0; j <= numIterations; j += 2)
        {
            float2 p0 = fontData.points[j + contour.start];
            float2 p1 = fontData.points[j + 1 + contour.start];
            float2 p2 = fontData.points[(j + 2) % (numIterations + 1) + contour.start];

            float2 point = input.uv + float2(input.pixelSize.x / 2.0f, 0.0f);
            if (checkHeightRange(point, p0, p1, p2))
            {
                HorizontalDistanceInfo info = getHorizontalDistance(point, p0, p1, p2);
                if (info.leftDist < 0.0f && info.leftDist > bottomLeft.dist)
                {
                    bottomLeft.dist = info.leftDist;
                    bottomLeft.yDir = info.leftTangent.y > 0.0f;
                    bottomLeft.valid = true;
                }
                else if (info.rightDist > 0.0f && info.rightDist < bottomRight.dist)
                {
                    bottomRight.dist = info.rightDist;
                    bottomRight.yDir = info.rightTangent.y > 0.0f;
                    bottomRight.valid = true;
                }
            }

            point.y += input.pixelSize.y - 1e-3f;
            if (checkHeightRange(point, p0, p1, p2))
            {
                HorizontalDistanceInfo info = getHorizontalDistance(point, p0, p1, p2);
                if (info.leftDist < 0.0f && info.leftDist > topLeft.dist)
                {
                    topLeft.dist = info.leftDist;
                    topLeft.yDir = info.leftTangent.y > 0.0f;
                    topLeft.valid = true;
                }
                else if (info.rightDist > 0.0f && info.rightDist < topRight.dist)
                {
                    topRight.dist = info.rightDist;
                    topRight.yDir = info.rightTangent.y > 0.0f;
                    topRight.valid = true;
                }
            }

            area += p0.x * p2.y - p2.x * p0.y;
        }

        bool goingClockWise = area < -1e-6f;

        if (bottomLeft.valid && bottomRight.valid &&
            min(abs(bottomLeft.dist), bottomRight.dist) < min(abs(closestBottomLeft.dist), closestBottomRight.dist) &&
            (bottomLeft.yDir != bottomRight.yDir || goingClockWise))
        {
            closestBottomLeft.dist = bottomLeft.dist;
            closestBottomLeft.yDir = bottomLeft.yDir;
            closestBottomLeft.valid = true;

            closestBottomRight.dist = bottomRight.dist;
            closestBottomRight.yDir = bottomRight.yDir;
            closestBottomRight.valid = true;
        }

        if (topLeft.valid && topRight.valid &&
            min(abs(topLeft.dist), topRight.dist) < min(abs(closestTopLeft.dist), closestTopRight.dist) &&
            (topLeft.yDir != topRight.yDir || goingClockWise))
        {
            closestTopLeft.dist = topLeft.dist;
            closestTopLeft.yDir = topLeft.yDir;
            closestTopLeft.valid = true;

            closestTopRight.dist = topRight.dist;
            closestTopRight.yDir = topRight.yDir;
            closestTopRight.valid = true;
        }
    }

    // Calculate pixel coverage
    float halfPixelWidth = input.pixelSize.x / 2.0f;
    float bottomCoverage = 0.0f;
    if (closestBottomLeft.valid && (closestBottomLeft.yDir || !closestBottomRight.yDir))
    {
        bottomCoverage += min(halfPixelWidth, abs(closestBottomLeft.dist)) / input.pixelSize.x;
    }
    if (closestBottomRight.valid && (closestBottomLeft.yDir || !closestBottomRight.yDir))
    {
        bottomCoverage += min(halfPixelWidth, closestBottomRight.dist) / input.pixelSize.x;
    }

    float topCoverage = 0.0f;
    if (closestTopLeft.valid && (closestTopLeft.yDir || !closestTopRight.yDir))
    {
        topCoverage += min(halfPixelWidth, abs(closestTopLeft.dist)) / input.pixelSize.x;
    }
    if (closestTopRight.valid && (closestTopLeft.yDir || !closestTopRight.yDir))
    {
        topCoverage += min(halfPixelWidth, closestTopRight.dist) / input.pixelSize.x;
    }

    float coverage = 0.0f;
    if (bottomCoverage != 0.0f && topCoverage != 0.0f)
    {
        coverage = (bottomCoverage + topCoverage) / 2.0f;
    }
    else
    {
        coverage = max(bottomCoverage, topCoverage) / 2.0f;
    }

    if (coverage == 0.0f)
    {
        discard;
    }

    output.outColor = float4(float3(coverage), 1.0f);
    return output;
}
