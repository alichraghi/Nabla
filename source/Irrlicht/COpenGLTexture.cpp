// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "irr/core/Types.h"
#include "COpenGLTexture.h"
#include "COpenGLDriver.h"
#include "os.h"
#include "CColorConverter.h"



namespace irr
{
namespace video
{


//! constructor for basic setup (only for derived classes)
COpenGLTexture::COpenGLTexture(const GLenum& textureType_Target)
	: TextureName(0), TextureNameHasChanged(0)
{
    COpenGLExtensionHandler::extGlCreateTextures(textureType_Target,1,&TextureName);

#ifdef OPENGL_LEAK_DEBUG
    COpenGLExtensionHandler::textureLeaker.registerObj(this);
#endif // OPENGL_LEAK_DEBUG
}


//! destructor
COpenGLTexture::~COpenGLTexture()
{
	if (TextureName)
		glDeleteTextures(1, &TextureName);

#ifdef OPENGL_LEAK_DEBUG
    COpenGLExtensionHandler::textureLeaker.deregisterObj(this);
#endif // OPENGL_LEAK_DEBUG
}

void COpenGLTexture::recreateName(const GLenum& textureType_Target)
{
    if (TextureName)
        glDeleteTextures(1, &TextureName);
    COpenGLExtensionHandler::extGlCreateTextures(textureType_Target,1,&TextureName);
	TextureNameHasChanged = CNullDriver::incrementAndFetchReallocCounter();
}



//! constructor for basic setup (only for derived classes)
COpenGLFilterableTexture::COpenGLFilterableTexture(const io::path& name, const GLenum& textureType_Target)
                                : ITexture(IDriverMemoryBacked::SDriverMemoryRequirements{{0,0,0},0,0,1,1},name), COpenGLTexture(textureType_Target), ColorFormat(EF_UNKNOWN),
                                InternalFormat(GL_RGBA), MipLevelsStored(0)
{
    TextureSize[0] = 1;
    TextureSize[1] = 1;
    TextureSize[2] = 1;
	#ifdef _DEBUG
	setDebugName("COpenGLFilterableTexture");
	#endif
}


//! Regenerates the mip map levels of the texture. Useful after locking and
//! modifying the texture
void COpenGLFilterableTexture::regenerateMipMapLevels()
{
	if (MipLevelsStored<=1)
		return;

    COpenGLExtensionHandler::extGlGenerateTextureMipmap(TextureName,this->getOpenGLTextureType());
}


uint32_t COpenGLTexture::getOpenGLFormatBpp(const GLenum& colorformat)
{
    switch(colorformat)
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            return 4;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return 8;
            break;
        case GL_COMPRESSED_RED_RGTC1_EXT:
            return 4;
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
            return 8;
            break;
        case GL_STENCIL_INDEX8:
        case GL_RGBA2:
        case GL_R3_G3_B2:
        case GL_R8:
        case GL_R8I:
        case GL_R8UI:
        case GL_R8_SNORM:
            return 8;
            break;
        case GL_RGB4:
            return 12;
            break;
        case GL_RGB5:
            return 15;
            break;
        case GL_DEPTH_COMPONENT16:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RG8:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG8_SNORM:
        case GL_R16:
        case GL_R16I:
        case GL_R16UI:
        case GL_R16_SNORM:
        case GL_R16F:
            return 16;
            break;
        case GL_DEPTH_COMPONENT24:
        case GL_RGB8:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB8_SNORM:
        case GL_SRGB8:
            return 24;
            break;
        case GL_RGB10:
            return 30;
            break;
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
        case GL_RGBA8:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA8_SNORM:
        case GL_SRGB8_ALPHA8:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RG16:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG16F:
        case GL_R32I:
        case GL_R32UI:
        case GL_R32F:
            return 32;
            break;
        case GL_RGB12:
            return 36;
            break;
        case GL_DEPTH32F_STENCIL8:
            return 40;
            break;
        case GL_RGBA12:
        case GL_RGB16:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB16_SNORM:
        case GL_RGB16F:
            return 48;
            break;
        case GL_RGBA16:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA16F:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RG32F:
            return 64;
            break;
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGB32F:
            return 96;
            break;
        case GL_RGBA32I:
        case GL_RGBA32UI:
        case GL_RGBA32F:
            return 128;
            break;
        default:
            return 0xdeadu;
    }
    return 0xdeadu;
}


bool COpenGLTexture::isInternalFormatCompressed(GLenum format)
{
    switch (format)
    {
        case GL_COMPRESSED_ALPHA:
        case GL_COMPRESSED_INTENSITY:
        case GL_COMPRESSED_LUMINANCE:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_R11_EAC:
        case GL_COMPRESSED_RED:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_RG:
        case GL_COMPRESSED_RG11_EAC:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGB8_ETC2:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
        case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
        case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
        case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
        case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
        case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
        case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
        case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
        case GL_COMPRESSED_RGBA_BPTC_UNORM:
        case GL_COMPRESSED_RGBA_FXT1_3DFX:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
        case GL_COMPRESSED_RGB_FXT1_3DFX:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_R11_EAC:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RG11_EAC:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_SLUMINANCE:
        case GL_COMPRESSED_SLUMINANCE_ALPHA:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        case GL_COMPRESSED_SRGB8_ETC2:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
            return true;
            break;
    }
    return false;
}

//! Get opengl values for the GPU texture storage
void COpenGLTexture::getOpenGLFormatAndParametersFromColorFormat(const E_FORMAT &format,
				GLenum& colorformat,
				GLenum& type)
{
	// default
	colorformat = GL_RGBA;
	type = GL_UNSIGNED_BYTE;

	switch(format)
	{
		case EF_A1R5G5B5_UNORM_PACK16:
			colorformat=GL_BGRA_EXT;
			type=GL_UNSIGNED_SHORT_1_5_5_5_REV;
			break;
		case EF_R5G6B5_UNORM_PACK16:
			colorformat=GL_RGB;
			type=GL_UNSIGNED_SHORT_5_6_5;
            break;
		// Floating Point texture formats. Thanks to Patryk "Nadro" Nadrowski.
		case EF_B10G11R11_UFLOAT_PACK32:
		{
			colorformat = GL_RGB;
			type = GL_R11F_G11F_B10F;
		}
			break;
		case EF_R16_SFLOAT:
		{
			colorformat = GL_RED;
			type = GL_HALF_FLOAT;
		}
			break;
		case EF_R16G16_SFLOAT:
		{
			colorformat = GL_RG;
			type = GL_HALF_FLOAT;
		}
			break;
		case EF_R16G16B16A16_SFLOAT:
		{
			colorformat = GL_RGBA;
			type = GL_HALF_FLOAT;
		}
			break;
		case EF_R32_SFLOAT:
		{
			colorformat = GL_RED;
			type = GL_FLOAT;
		}
			break;
		case EF_R32G32_SFLOAT:
		{
			colorformat = GL_RG;
			type = GL_FLOAT;
		}
			break;
		case EF_R32G32B32A32_SFLOAT:
		{
			colorformat = GL_RGBA;
			type = GL_FLOAT;
		}
			break;
        case EF_R8_SNORM:
		case EF_R8_UNORM:
		{
			colorformat = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}
			break;
        case EF_R8G8_SNORM:
		case EF_R8G8_UNORM:
		{
			colorformat = GL_RG;
			type = GL_UNSIGNED_BYTE;
		}
			break;
        case EF_R8G8B8_SNORM:
        case EF_R8G8B8_UNORM:
            colorformat = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
        case EF_B8G8R8A8_SNORM:
        case EF_B8G8R8A8_UNORM:
            colorformat = GL_BGRA_EXT;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
        case EF_R8G8B8A8_SNORM:
        case EF_R8G8B8A8_UNORM:
            colorformat = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case EF_R8_UINT:
        {
            colorformat = GL_RED_INTEGER;
            type = GL_UNSIGNED_BYTE;
        }
        break;
        case EF_R8G8_UINT:
        {
            colorformat = GL_RG_INTEGER;
            type = GL_UNSIGNED_BYTE;
        }
        break;
        case EF_R8G8B8_UINT:
        {
            colorformat = GL_RGB_INTEGER;
            type = GL_UNSIGNED_BYTE;
        }
        break;
        case EF_R8G8B8A8_UINT:
        {
            colorformat = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_BYTE;
        }
        break;
        case EF_R8_SINT:
        {
            colorformat = GL_RED_INTEGER;
            type = GL_BYTE;
        }
        break;
        case EF_R8G8_SINT:
        {
            colorformat = GL_RG_INTEGER;
            type = GL_BYTE;
        }
        break;
        case EF_R8G8B8_SINT:
        {
            colorformat = GL_RGB_INTEGER;
            type = GL_BYTE;
        }
        break;
        case EF_R8G8B8A8_SINT:
        {
            colorformat = GL_RGBA_INTEGER;
            type = GL_BYTE;
        }
        case EF_R16_SNORM:
        case EF_R16_UNORM:
        {
            colorformat = GL_RED;
            type = GL_UNSIGNED_SHORT;
        }
        break;
        case EF_R16G16_SNORM:
        case EF_R16G16_UNORM:
        {
            colorformat = GL_RG;
            type = GL_UNSIGNED_SHORT;
        }
        break;
        case EF_R16G16B16_SNORM:
        case EF_R16G16B16_UNORM:
        {
            colorformat = GL_RGB;
            type = GL_UNSIGNED_SHORT;
        }
            break;
        case EF_R16G16B16A16_SNORM:
        case EF_R16G16B16A16_UNORM:
        {
            colorformat = GL_RGBA;
            type = GL_UNSIGNED_SHORT;
        }
            break;
        case EF_R16_UINT:
        {
            colorformat = GL_RED_INTEGER;
            type = GL_UNSIGNED_SHORT;
        }
        break;
        case EF_R16G16_UINT:
        {
            colorformat = GL_RG_INTEGER;
            type = GL_UNSIGNED_SHORT;
        }
        break;
        case EF_R16G16B16_UINT:
        {
            colorformat = GL_RGB_INTEGER;
            type = GL_UNSIGNED_SHORT;
        }
        break;
        case EF_R16G16B16A16_UINT:
        {
            colorformat = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_SHORT;
        }
        break;
        case EF_R16_SINT:
        {
            colorformat = GL_RED_INTEGER;
            type = GL_SHORT;
        }
        break;
        case EF_R16G16_SINT:
        {
            colorformat = GL_RG_INTEGER;
            type = GL_SHORT;
        }
        break;
        case EF_R16G16B16_SINT:
        {
            colorformat = GL_RGB_INTEGER;
            type = GL_SHORT;
        }
        break;
        case EF_R16G16B16A16_SINT:
        {
            colorformat = GL_RGBA_INTEGER;
            type = GL_SHORT;
        }
        break;
        case EF_R32_UINT:
        {
            colorformat = GL_RED_INTEGER;
            type = GL_UNSIGNED_INT;
        }
        break;
        case EF_R32G32_UINT:
        {
            colorformat = GL_RG_INTEGER;
            type = GL_UNSIGNED_INT;
        }
        break;
        case EF_R32G32B32_UINT:
        {
            colorformat = GL_RGB_INTEGER;
            type = GL_UNSIGNED_INT;
        }
        break;
        case EF_R32G32B32A32_UINT:
        {
            colorformat = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_INT;
        }
        break;
        case EF_R32_SINT:
        {
            colorformat = GL_RED_INTEGER;
            type = GL_INT;
        }
        break;
        case EF_R32G32_SINT:
        {
            colorformat = GL_RG_INTEGER;
            type = GL_INT;
        }
        break;
        case EF_R32G32B32_SINT:
        {
            colorformat = GL_RGB_INTEGER;
            type = GL_INT;
        }
        break;
        case EF_R32G32B32A32_SINT:
        {
            colorformat = GL_RGBA_INTEGER;
            type = GL_INT;
        }
        break;
		case EF_BC1_RGB_UNORM_BLOCK:
		{
			colorformat = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_BC1_RGBA_UNORM_BLOCK:
		{
			colorformat = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_BC2_UNORM_BLOCK:
		{
			colorformat = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_BC3_UNORM_BLOCK:
		{
			colorformat = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
			break;
        /* // todo bc
		case EF_R_BC4:
		{
			colorformat = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_RG_BC5:
		{
			colorformat = GL_RG;
			type = GL_UNSIGNED_BYTE;
		}
			break;
         */
		case EF_8BIT_PIX:
		{
			colorformat = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_16BIT_PIX:
		{
			colorformat = GL_RG;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_24BIT_PIX:
		{
			colorformat = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_32BIT_PIX:
		{
			colorformat = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_48BIT_PIX:
		{
			colorformat = GL_RGB;
			type = GL_UNSIGNED_SHORT;
		}
			break;
		case EF_64BIT_PIX:
		{
			colorformat = GL_RGBA;
			type = GL_UNSIGNED_SHORT;
		}
			break;
		case EF_96BIT_PIX:
		{
			colorformat = GL_RGB;
			type = GL_FLOAT;
		}
			break;
		case EF_128BIT_PIX:
		{
			colorformat = GL_RGBA;
			type = GL_FLOAT;
		}
			break;
        /// this is totally wrong but safe - most probs have to reupload
		case EF_DEPTH16:
		{
			colorformat = GL_DEPTH;
			type = GL_UNSIGNED_SHORT;
		}
			break;
		case EF_DEPTH24:
		{
			colorformat = GL_DEPTH;
			type = GL_UNSIGNED_SHORT;
		}
			break;
		case EF_DEPTH24_STENCIL8:
		{
			colorformat = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_INT_24_8_EXT;
		}
			break;
		case EF_DEPTH32F:
		{
			colorformat = GL_DEPTH;
			type = GL_FLOAT;
		}
			break;
		case EF_DEPTH32F_STENCIL8:
		{
			colorformat = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_BYTE;
		}
			break;
		case EF_STENCIL8:
		{
			colorformat = GL_STENCIL;
			type = GL_UNSIGNED_BYTE;
		}
			break;
        case EF_E5B9G9R9_UFLOAT_PACK32:
        {
            colorformat = GL_RGB;
            type = GL_HALF_FLOAT;
        }
            break;
		default:
		{
			os::Printer::log("Unsupported upload format", ELL_ERROR);
		}
	}
}

GLint COpenGLTexture::getOpenGLFormatAndParametersFromColorFormat(const E_FORMAT &format)
{
	switch(format)
	{
		case EF_A1R5G5B5_UNORM_PACK16:
			return GL_RGB5_A1;
			break;
		case EF_R5G6B5_UNORM_PACK16:
			return GL_RGB565;
			break;
		// Floating Point texture formats. Thanks to Patryk "Nadro" Nadrowski.
		case EF_B10G11R11_UFLOAT_PACK32:
            return GL_R11F_G11F_B10F;
			break;
		case EF_R16_SFLOAT:
		    return GL_R16F;
			break;
		case EF_R16G16_SFLOAT:
		    return GL_RG16F;
			break;
        case EF_R16G16B16_SFLOAT:
            return GL_RGB16F;
		case EF_R16G16B16A16_SFLOAT:
		    return GL_RGBA16F;
			break;
		case EF_R32_SFLOAT:
		    return GL_R32F;
			break;
		case EF_R32G32_SFLOAT:
		    return GL_RG32F;
			break;
        case EF_R32G32B32_SFLOAT:
            return GL_RGB32F;
            break;
		case EF_R32G32B32A32_SFLOAT:
		    return GL_RGBA32F;
			break;
		case EF_R8_UNORM:
		    return GL_R8;
			break;
		case EF_R8G8_UNORM:
		    return GL_RG8;
			break;
        case EF_R8G8B8_UNORM:
            return GL_RGB8;
            break;
        case EF_B8G8R8A8_UNORM:
            return GL_RGBA8;
            break;
        case EF_R8G8B8A8_UNORM:
            return GL_RGBA8;
            break;
        case EF_R8_UINT:
            return GL_R8UI;
            break;
        case EF_R8G8_UINT:
            return GL_RG8UI;
            break;
        case EF_R8G8B8_UINT:
            return GL_RGB8UI;
            break;
        case EF_B8G8R8A8_UINT:
            return GL_RGBA8UI;
            break;
        case EF_R8G8B8A8_UINT:
            return GL_RGBA8UI;
            break;
        case EF_R8_SINT:
            return GL_R8I;
            break;
        case EF_R8G8_SINT:
            return GL_RG8I;
            break;
        case EF_R8G8B8_SINT:
            return GL_RGB8I;
            break;
        case EF_B8G8R8A8_SINT:
            return GL_RGBA8I;
            break;
        case EF_R8G8B8A8_SINT:
            return GL_RGBA8I;
            break;
        case EF_R8_SNORM:
            return GL_R8_SNORM;
            break;
        case EF_R8G8_SNORM:
            return GL_RG8_SNORM;
            break;
        case EF_R8G8B8_SNORM:
            return GL_RGB8_SNORM;
            break;
        case EF_B8G8R8A8_SNORM:
            return GL_RGBA8_SNORM;
            break;
        case EF_R8G8B8A8_SNORM:
            return GL_RGBA8_SNORM;
            break;
        case EF_R16_UNORM:
            return GL_R16;
            break;
        case EF_R16G16_UNORM:
            return GL_RG16;
            break;
        case EF_R16G16B16_UNORM:
            return GL_RGB16;
            break;
        case EF_R16G16B16A16_UNORM:
            return GL_RGBA16;
            break;
        case EF_R16_UINT:
            return GL_R16UI;
            break;
        case EF_R16G16_UINT:
            return GL_RG16UI;
            break;
        case EF_R16G16B16_UINT:
            return GL_RGB16UI;
            break;
        case EF_R16G16B16A16_UINT:
            return GL_RGBA16UI;
            break;
        case EF_R16_SINT:
            return GL_R16I;
            break;
        case EF_R16G16_SINT:
            return GL_RG16I;
            break;
        case EF_R16G16B16_SINT:
            return GL_RGB16I;
            break;
        case EF_R16G16B16A16_SINT:
            return GL_RGBA16I;
            break;
        case EF_R16_SNORM:
            return GL_R16_SNORM;
            break;
        case EF_R16G16_SNORM:
            return GL_RG16_SNORM;
            break;
        case EF_R16G16B16_SNORM:
            return GL_RGB16_SNORM;
            break;
        case EF_R16G16B16A16_SNORM:
            return GL_RGBA16_SNORM;
            break;
        case EF_R32_UINT:
            return GL_R32UI;
            break;
        case EF_R32G32_UINT:
            return GL_RG32UI;
            break;
        case EF_R32G32B32_UINT:
            return GL_RGB32UI;
            break;
        case EF_R32G32B32A32_UINT:
            return GL_RGBA32UI;
            break;
        case EF_R32_SINT:
            return GL_R32I;
            break;
        case EF_R32G32_SINT:
            return GL_RG32I;
            break;
        case EF_R32G32B32_SINT:
            return GL_RGB32I;
            break;
        case EF_R32G32B32A32_SINT:
            return GL_RGBA32I;
            break;
        case EF_R8G8B8_SRGB:
            return GL_SRGB8;
            break;
        case EF_R8G8B8A8_SRGB:
            return GL_SRGB8_ALPHA8;
            break;
		case EF_BC1_RGB_UNORM_BLOCK:
		    return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			break;
		case EF_BC1_RGBA_UNORM_BLOCK:
		    return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		case EF_BC2_UNORM_BLOCK:
		    return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case EF_BC3_UNORM_BLOCK:
		    return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
            /* // todo bc
		case EF_R_BC4:
		    return GL_COMPRESSED_RED_RGTC1_EXT;
			break;
		case EF_RG_BC5:
		    return GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
			break;
            */
		case EF_8BIT_PIX:
		    return GL_R8;
			break;
		case EF_16BIT_PIX:
		    return GL_RG8;
			break;
		case EF_24BIT_PIX:
		    return GL_RGB8;
			break;
		case EF_32BIT_PIX:
		    return GL_RGBA8;
			break;
		case EF_48BIT_PIX:
		    return GL_RGB16;
			break;
		case EF_64BIT_PIX:
		    return GL_RGBA16;
			break;
		case EF_96BIT_PIX:
		    return GL_RGB32F;
			break;
		case EF_128BIT_PIX:
		    return GL_RGBA32F;
			break;
        /// this is totally wrong but safe - most probs have to reupload
		case EF_DEPTH16:
		    return GL_DEPTH_COMPONENT16;
			break;
		case EF_DEPTH24:
		    return GL_DEPTH_COMPONENT24;
			break;
		case EF_DEPTH24_STENCIL8:
		    return GL_DEPTH24_STENCIL8;
			break;
		case EF_DEPTH32F:
		    return GL_DEPTH_COMPONENT32F;
			break;
		case EF_DEPTH32F_STENCIL8:
		    return GL_DEPTH32F_STENCIL8;
			break;
		case EF_STENCIL8:
            return GL_STENCIL_INDEX8;
			break;
        case EF_E5B9G9R9_UFLOAT_PACK32:
            return GL_RGB9_E5;
            break;
		default:
		{
#ifdef _DEBUG
			os::Printer::log("Unsupported texture format", ELL_ERROR);
#endif // _DEBUG
			return GL_INVALID_ENUM;
		}
	}
}

E_FORMAT COpenGLTexture::getColorFormatFromSizedOpenGLFormat(const GLenum& sizedFormat)
{
    switch(sizedFormat)
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            return EF_BC1_RGB_UNORM_BLOCK;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            return EF_BC1_RGBA_UNORM_BLOCK;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            return EF_BC2_UNORM_BLOCK;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return EF_BC3_UNORM_BLOCK;
            break;
            /* // todo bc
        case GL_COMPRESSED_RED_RGTC1_EXT:
            return EF_R_BC4;
            break;
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
            return EF_RG_BC5;
            break;
            */
        case GL_STENCIL_INDEX8:
            return EF_STENCIL8;
            break;
        case GL_RGBA2:
            ///return EF_8BIT_PIX;
            break;
        case GL_R3_G3_B2:
            ///return EF_8BIT_PIX;
            break;
        case GL_R8:
            return EF_R8_UNORM;
            break;
        case GL_R8I:
            return EF_R8_SINT;
            break;
        case GL_R8UI:
            return EF_R8_UINT;
            break;
        case GL_R8_SNORM:
            return EF_R8_SNORM;
            break;
        case GL_RGB4:
            ///return EF_16BIT_PIX;
            break;
        case GL_RGB5:
            ///return EF_;
            break;
        case GL_DEPTH_COMPONENT16:
            return EF_DEPTH16;
            break;
        case GL_RGBA4:
            return EF_R4G4B4A4_UNORM_PACK16;
            break;
        case GL_RGB5_A1:
            return EF_R5G5B5A1_UNORM_PACK16;
            break;
        case GL_RG8:
            return EF_R8G8_UNORM;
            break;
        case GL_RG8I:
            return EF_R8G8_SINT;
            break;
        case GL_RG8UI:
            return EF_R8G8_UINT;
            break;
        case GL_RG8_SNORM:
            return EF_R8G8_SNORM;
            break;
        case GL_R16:
            return EF_R16_UNORM;
            break;
        case GL_R16I:
            return EF_R16_SINT;
            break;
        case GL_R16UI:
            return EF_R16_UINT;
            break;
        case GL_R16_SNORM:
            return EF_R16_SNORM;
            break;
        case GL_R16F:
            return EF_R16_SFLOAT;
            break;
        case GL_DEPTH_COMPONENT24:
            return EF_DEPTH24;
            break;
        case GL_RGB8:
            return EF_R8G8B8_UNORM;
            break;
        case GL_RGB8I:
            return EF_R8G8B8_SINT;
            break;
        case GL_RGB8UI:
            return EF_R8G8B8_UINT;
            break;
        case GL_RGB8_SNORM:
            return EF_R8G8B8_SNORM;
            break;
        case GL_SRGB8:
            return EF_R8G8B8_SRGB;
            break;
        case GL_RGB10:
            ///return EF_;
            break;
        case GL_DEPTH24_STENCIL8:
            return EF_DEPTH24_STENCIL8;
            break;
        case GL_DEPTH_COMPONENT32:
            ///return EF_DEPTH32;
            break;
        case GL_DEPTH_COMPONENT32F:
            return EF_DEPTH32F;
            break;
        case GL_RGBA8:
            return EF_R8G8B8A8_UNORM;
            break;
        case GL_RGBA8I:
            return EF_R8G8B8A8_SINT;
            break;
        case GL_RGBA8UI:
            return EF_R8G8B8A8_UINT;
            break;
        case GL_RGBA8_SNORM:
            return EF_R8G8B8A8_SNORM;
            break;
        case GL_SRGB8_ALPHA8:
            return EF_R8G8B8A8_SRGB;
            break;
        case GL_RGB10_A2:
            return EF_A2B10G10R10_UNORM_PACK32;
            break;
        case GL_RGB10_A2UI:
            return EF_A2B10G10R10_UINT_PACK32;
            break;
        case GL_R11F_G11F_B10F:
            return EF_B10G11R11_UFLOAT_PACK32;
            break;
        case GL_RGB9_E5:
            return EF_E5B9G9R9_UFLOAT_PACK32;
            break;
        case GL_RG16:
            return EF_R16G16_UNORM;
            break;
        case GL_RG16I:
            return EF_R16G16_SINT;
            break;
        case GL_RG16UI:
            return EF_R16G16_UINT;
            break;
        case GL_RG16F:
            return EF_R16G16_SFLOAT;
            break;
        case GL_R32I:
            return EF_R32G32_SINT;
            break;
        case GL_R32UI:
            return EF_R32G32_UINT;
            break;
        case GL_R32F:
            return EF_R32_SFLOAT;
            break;
        case GL_RGB12:
            ///return EF_;
            break;
        case GL_DEPTH32F_STENCIL8:
            return EF_DEPTH32F_STENCIL8;
            break;
        case GL_RGBA12:
            ///return EF_;
            break;
        case GL_RGB16:
            return EF_R16G16B16_UNORM;
            break;
        case GL_RGB16I:
            return EF_R16G16B16_SINT;
            break;
        case GL_RGB16UI:
            return EF_R16G16B16_UINT;
            break;
        case GL_RGB16_SNORM:
            return EF_R16G16B16_SNORM;
            break;
        case GL_RGB16F:
            return EF_R16G16B16_SFLOAT;
            break;
        case GL_RGBA16:
            return EF_R16G16B16A16_UNORM;
            break;
        case GL_RGBA16I:
            return EF_R16G16B16A16_SINT;
            break;
        case GL_RGBA16UI:
            return EF_R16G16B16A16_UINT;
            break;
        case GL_RGBA16F:
            return EF_R16G16B16A16_SFLOAT;
            break;
        case GL_RG32I:
            return EF_R32G32_SINT;
            break;
        case GL_RG32UI:
            return EF_R32G32_UINT;
            break;
        case GL_RG32F:
            return EF_R32G32_SFLOAT;
            break;
        case GL_RGB32I:
            return EF_R32G32B32_SINT;
            break;
        case GL_RGB32UI:
            return EF_R32G32B32_UINT;
            break;
        case GL_RGB32F:
            return EF_R32G32B32_SFLOAT;
            break;
        case GL_RGBA32I:
            return EF_R32G32B32A32_SINT;
            break;
        case GL_RGBA32UI:
            return EF_R32G32B32A32_UINT;
            break;
        case GL_RGBA32F:
            return EF_R32G32B32A32_SFLOAT;
            break;
        default:
            break;
    }
    return EF_UNKNOWN;
}


} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_OPENGL_

