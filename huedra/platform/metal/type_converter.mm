#include "type_converter.hpp"

namespace huedra::converter {

MTLVertexFormat convertDataFormat(GraphicsDataFormat format)
{
    switch (format)
    {
    case GraphicsDataFormat::UNDEFINED:
        return MTLVertexFormatInvalid;

    case GraphicsDataFormat::R_8_INT:
        return MTLVertexFormatChar;
    case GraphicsDataFormat::R_8_UINT:
        return MTLVertexFormatUChar;
    case GraphicsDataFormat::R_8_NORM:
        return MTLVertexFormatCharNormalized;
    case GraphicsDataFormat::R_8_UNORM:
        return MTLVertexFormatUCharNormalized;
    case GraphicsDataFormat::R_16_INT:
        return MTLVertexFormatShort;
    case GraphicsDataFormat::R_16_UINT:
        return MTLVertexFormatUShort;
    case GraphicsDataFormat::R_16_FLOAT:
        return MTLVertexFormatHalf;
    case GraphicsDataFormat::R_16_NORM:
        return MTLVertexFormatShortNormalized;
    case GraphicsDataFormat::R_16_UNORM:
        return MTLVertexFormatUShortNormalized;
    case GraphicsDataFormat::R_32_INT:
        return MTLVertexFormatInt;
    case GraphicsDataFormat::R_32_UINT:
        return MTLVertexFormatUInt;
    case GraphicsDataFormat::R_32_FLOAT:
        return MTLVertexFormatFloat;
    case GraphicsDataFormat::R_64_INT:
    case GraphicsDataFormat::R_64_UINT:
    case GraphicsDataFormat::R_64_FLOAT:
        return MTLVertexFormatInvalid;

    case GraphicsDataFormat::RG_8_INT:
        return MTLVertexFormatChar2;
    case GraphicsDataFormat::RG_8_UINT:
        return MTLVertexFormatUChar2;
    case GraphicsDataFormat::RG_8_NORM:
        return MTLVertexFormatChar2Normalized;
    case GraphicsDataFormat::RG_8_UNORM:
        return MTLVertexFormatUChar2Normalized;
    case GraphicsDataFormat::RG_16_INT:
        return MTLVertexFormatShort2;
    case GraphicsDataFormat::RG_16_UINT:
        return MTLVertexFormatUShort2;
    case GraphicsDataFormat::RG_16_FLOAT:
        return MTLVertexFormatHalf2;
    case GraphicsDataFormat::RG_16_NORM:
        return MTLVertexFormatShort2Normalized;
    case GraphicsDataFormat::RG_16_UNORM:
        return MTLVertexFormatUShort2Normalized;
    case GraphicsDataFormat::RG_32_INT:
        return MTLVertexFormatInt2;
    case GraphicsDataFormat::RG_32_UINT:
        return MTLVertexFormatUInt2;
    case GraphicsDataFormat::RG_32_FLOAT:
        return MTLVertexFormatFloat2;
    case GraphicsDataFormat::RG_64_INT:
    case GraphicsDataFormat::RG_64_UINT:
    case GraphicsDataFormat::RG_64_FLOAT:
        return MTLVertexFormatInvalid;

    case GraphicsDataFormat::RGB_8_INT:
        return MTLVertexFormatChar3;
    case GraphicsDataFormat::RGB_8_UINT:
        return MTLVertexFormatUChar3;
    case GraphicsDataFormat::RGB_8_NORM:
        return MTLVertexFormatChar3Normalized;
    case GraphicsDataFormat::RGB_8_UNORM:
        return MTLVertexFormatUChar3Normalized;
    case GraphicsDataFormat::RGB_16_INT:
        return MTLVertexFormatShort3;
    case GraphicsDataFormat::RGB_16_UINT:
        return MTLVertexFormatUShort3;
    case GraphicsDataFormat::RGB_16_FLOAT:
        return MTLVertexFormatHalf3;
    case GraphicsDataFormat::RGB_16_NORM:
        return MTLVertexFormatShort3Normalized;
    case GraphicsDataFormat::RGB_16_UNORM:
        return MTLVertexFormatUShort3Normalized;
    case GraphicsDataFormat::RGB_32_INT:
        return MTLVertexFormatInt3;
    case GraphicsDataFormat::RGB_32_UINT:
        return MTLVertexFormatUInt3;
    case GraphicsDataFormat::RGB_32_FLOAT:
        return MTLVertexFormatFloat3;
    case GraphicsDataFormat::RGB_64_INT:
    case GraphicsDataFormat::RGB_64_UINT:
    case GraphicsDataFormat::RGB_64_FLOAT:
        return MTLVertexFormatInvalid;

    case GraphicsDataFormat::RGBA_8_INT:
        return MTLVertexFormatChar4;
    case GraphicsDataFormat::RGBA_8_UINT:
        return MTLVertexFormatUChar4;
    case GraphicsDataFormat::RGBA_8_NORM:
        return MTLVertexFormatChar4Normalized;
    case GraphicsDataFormat::RGBA_8_UNORM:
        return MTLVertexFormatUChar4Normalized;
    case GraphicsDataFormat::RGBA_16_INT:
        return MTLVertexFormatShort4;
    case GraphicsDataFormat::RGBA_16_UINT:
        return MTLVertexFormatUShort4;
    case GraphicsDataFormat::RGBA_16_FLOAT:
        return MTLVertexFormatHalf4;
    case GraphicsDataFormat::RGBA_16_NORM:
        return MTLVertexFormatShort4Normalized;
    case GraphicsDataFormat::RGBA_16_UNORM:
        return MTLVertexFormatUShort4Normalized;
    case GraphicsDataFormat::RGBA_32_INT:
        return MTLVertexFormatInt4;
    case GraphicsDataFormat::RGBA_32_UINT:
        return MTLVertexFormatUInt4;
    case GraphicsDataFormat::RGBA_32_FLOAT:
        return MTLVertexFormatFloat4;
    case GraphicsDataFormat::RGBA_64_INT:
    case GraphicsDataFormat::RGBA_64_UINT:
    case GraphicsDataFormat::RGBA_64_FLOAT:
        return MTLVertexFormatInvalid;
    }
}

MTLVertexStepFunction convertVertexInputRate(VertexInputRate inputRate)
{
    switch (inputRate)
    {
    case VertexInputRate::VERTEX:
        return MTLVertexStepFunctionPerVertex;
    case VertexInputRate::INSTANCE:
        return MTLVertexStepFunctionPerInstance;
    }
}

} // namespace huedra::converter
