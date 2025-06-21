#include "type_converter.hpp"

namespace huedra::converter {

MTLPixelFormat convertPixelDataFormat(GraphicsDataFormat format)
{
    switch (format)
    {
    case GraphicsDataFormat::UNDEFINED:
        return MTLPixelFormatInvalid;

    case GraphicsDataFormat::R_8_INT:
        return MTLPixelFormatR8Sint;
    case GraphicsDataFormat::R_8_UINT:
        return MTLPixelFormatR8Uint;
    case GraphicsDataFormat::R_8_NORM:
        return MTLPixelFormatR8Snorm;
    case GraphicsDataFormat::R_8_UNORM:
        return MTLPixelFormatR8Unorm;
    case GraphicsDataFormat::R_16_INT:
        return MTLPixelFormatR16Sint;
    case GraphicsDataFormat::R_16_UINT:
        return MTLPixelFormatR16Uint;
    case GraphicsDataFormat::R_16_FLOAT:
        return MTLPixelFormatR16Float;
    case GraphicsDataFormat::R_16_NORM:
        return MTLPixelFormatR16Snorm;
    case GraphicsDataFormat::R_16_UNORM:
        return MTLPixelFormatR16Unorm;
    case GraphicsDataFormat::R_32_INT:
        return MTLPixelFormatR32Sint;
    case GraphicsDataFormat::R_32_UINT:
        return MTLPixelFormatR32Uint;
    case GraphicsDataFormat::R_32_FLOAT:
        return MTLPixelFormatR32Float;
    case GraphicsDataFormat::R_64_INT:
    case GraphicsDataFormat::R_64_UINT:
    case GraphicsDataFormat::R_64_FLOAT:
        return MTLPixelFormatInvalid;

    case GraphicsDataFormat::RG_8_INT:
        return MTLPixelFormatRG8Sint;
    case GraphicsDataFormat::RG_8_UINT:
        return MTLPixelFormatRG8Uint;
    case GraphicsDataFormat::RG_8_NORM:
        return MTLPixelFormatRG8Snorm;
    case GraphicsDataFormat::RG_8_UNORM:
        return MTLPixelFormatRG8Unorm;
    case GraphicsDataFormat::RG_16_INT:
        return MTLPixelFormatRG16Sint;
    case GraphicsDataFormat::RG_16_UINT:
        return MTLPixelFormatRG16Uint;
    case GraphicsDataFormat::RG_16_FLOAT:
        return MTLPixelFormatRG16Float;
    case GraphicsDataFormat::RG_16_NORM:
        return MTLPixelFormatRG16Snorm;
    case GraphicsDataFormat::RG_16_UNORM:
        return MTLPixelFormatRG16Unorm;
    case GraphicsDataFormat::RG_32_INT:
        return MTLPixelFormatRG32Sint;
    case GraphicsDataFormat::RG_32_UINT:
        return MTLPixelFormatRG32Uint;
    case GraphicsDataFormat::RG_32_FLOAT:
        return MTLPixelFormatRG32Float;
    case GraphicsDataFormat::RG_64_INT:
    case GraphicsDataFormat::RG_64_UINT:
    case GraphicsDataFormat::RG_64_FLOAT:
        return MTLPixelFormatInvalid;

    case GraphicsDataFormat::RGB_8_INT:
    case GraphicsDataFormat::RGB_8_UINT:
    case GraphicsDataFormat::RGB_8_NORM:
    case GraphicsDataFormat::RGB_8_UNORM:
    case GraphicsDataFormat::BGR_8_INT:
    case GraphicsDataFormat::BGR_8_UINT:
    case GraphicsDataFormat::BGR_8_NORM:
    case GraphicsDataFormat::BGR_8_UNORM:
    case GraphicsDataFormat::RGB_16_INT:
    case GraphicsDataFormat::RGB_16_UINT:
    case GraphicsDataFormat::RGB_16_FLOAT:
    case GraphicsDataFormat::RGB_16_NORM:
    case GraphicsDataFormat::RGB_16_UNORM:
    case GraphicsDataFormat::RGB_32_INT:
    case GraphicsDataFormat::RGB_32_UINT:
    case GraphicsDataFormat::RGB_32_FLOAT:
    case GraphicsDataFormat::RGB_64_INT:
    case GraphicsDataFormat::RGB_64_UINT:
    case GraphicsDataFormat::RGB_64_FLOAT:
        return MTLPixelFormatInvalid;

    case GraphicsDataFormat::RGBA_8_INT:
        return MTLPixelFormatRGBA8Sint;
    case GraphicsDataFormat::RGBA_8_UINT:
        return MTLPixelFormatRGBA8Uint;
    case GraphicsDataFormat::RGBA_8_NORM:
        return MTLPixelFormatRGBA8Snorm;
    case GraphicsDataFormat::RGBA_8_UNORM:
        return MTLPixelFormatRGBA8Unorm;
    case GraphicsDataFormat::BGRA_8_INT:
        return MTLPixelFormatBGRA8Sint;
    case GraphicsDataFormat::BGRA_8_UINT:
        return MTLPixelFormatBGRA8Uint;
    case GraphicsDataFormat::BGRA_8_NORM:
        return MTLPixelFormatBGRA8Snorm;
    case GraphicsDataFormat::BGRA_8_UNORM:
        return MTLPixelFormatBGRA8Unorm;
    case GraphicsDataFormat::RGBA_16_INT:
        return MTLPixelFormatRGBA16Sint;
    case GraphicsDataFormat::RGBA_16_UINT:
        return MTLPixelFormatRGBA16Uint;
    case GraphicsDataFormat::RGBA_16_FLOAT:
        return MTLPixelFormatRGBA16Float;
    case GraphicsDataFormat::RGBA_16_NORM:
        return MTLPixelFormatRGBA16Snorm;
    case GraphicsDataFormat::RGBA_16_UNORM:
        return MTLPixelFormatRGBA16Unorm;
    case GraphicsDataFormat::RGBA_32_INT:
        return MTLPixelFormatRGBA32Sint;
    case GraphicsDataFormat::RGBA_32_UINT:
        return MTLPixelFormatRGBA32Uint;
    case GraphicsDataFormat::RGBA_32_FLOAT:
        return MTLPixelFormatRGBA32Float;
    case GraphicsDataFormat::RGBA_64_INT:
    case GraphicsDataFormat::RGBA_64_UINT:
    case GraphicsDataFormat::RGBA_64_FLOAT:
        return MTLPixelFormatInvalid;
    }
}

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
    case GraphicsDataFormat::BGR_8_INT:
        return MTLVertexFormatChar3;
    case GraphicsDataFormat::RGB_8_UINT:
    case GraphicsDataFormat::BGR_8_UINT:
        return MTLVertexFormatUChar3;
    case GraphicsDataFormat::RGB_8_NORM:
    case GraphicsDataFormat::BGR_8_NORM:
        return MTLVertexFormatChar3Normalized;
    case GraphicsDataFormat::RGB_8_UNORM:
    case GraphicsDataFormat::BGR_8_UNORM:
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
    case GraphicsDataFormat::BGRA_8_INT:
        return MTLVertexFormatChar4;
    case GraphicsDataFormat::RGBA_8_UINT:
    case GraphicsDataFormat::BGRA_8_UINT:
        return MTLVertexFormatUChar4;
    case GraphicsDataFormat::RGBA_8_NORM:
    case GraphicsDataFormat::BGRA_8_NORM:
        return MTLVertexFormatChar4Normalized;
    case GraphicsDataFormat::RGBA_8_UNORM:
    case GraphicsDataFormat::BGRA_8_UNORM:
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

MTLPrimitiveType convertPrimitiveType(PrimitiveType type, PrimitiveLayout layout)
{
    if (type == PrimitiveType::POINT)
    {
        return MTLPrimitiveTypePoint;
    }

    switch (layout)
    {
    case PrimitiveLayout::POINT_LIST:
        return MTLPrimitiveTypePoint;
    case PrimitiveLayout::LINE_LIST:
        return MTLPrimitiveTypeLine;
    case PrimitiveLayout::LINE_STRIP:
        return MTLPrimitiveTypeLineStrip;
    case PrimitiveLayout::TRIANGLE_LIST:
        return MTLPrimitiveTypeTriangle;
    case PrimitiveLayout::TRIANGLE_STRIP:
        return MTLPrimitiveTypeTriangleStrip;
    }
}

MTLTriangleFillMode convertTriangleFillMode(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::POINT:
    case PrimitiveType::LINE:
        return MTLTriangleFillModeLines;
    case PrimitiveType::TRIANGLE:
        return MTLTriangleFillModeFill;
    }
}

} // namespace huedra::converter
