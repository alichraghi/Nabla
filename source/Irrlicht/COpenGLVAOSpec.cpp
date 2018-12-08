#include "IrrCompileConfig.h"
#include "COpenGLVAOSpec.h"


#ifdef _IRR_COMPILE_WITH_OPENGL_

namespace std
{
	template <>
	struct hash<irr::video::COpenGLVAOSpec::HashAttribs>
	{
		std::size_t operator()(const irr::video::COpenGLVAOSpec::HashAttribs& x) const noexcept
		{
			size_t retval = hash<uint64_t>()(x.hashVal[0]);

			for (size_t i = 1; i<irr::video::COpenGLVAOSpec::HashAttribs::getHashLength(); i++)
				retval ^= hash<uint64_t>()(x.hashVal[i]);

			return retval;
		}
	};
}

namespace irr
{
namespace video
{

COpenGLVAOSpec::COpenGLVAOSpec(core::LeakDebugger* dbgr) :  leakDebugger(dbgr)
{
    if (leakDebugger)
        leakDebugger->registerObj(this);

    for (size_t i=0; i<scene::EVAI_COUNT; i++)
        individualHashFields.setAttrFmt(static_cast<scene::E_VERTEX_ATTRIBUTE_ID>(i),attrFormat[i]);
}

COpenGLVAOSpec::~COpenGLVAOSpec()
{
    if (leakDebugger)
        leakDebugger->deregisterObj(this);
}


void COpenGLVAOSpec::mapVertexAttrBuffer(IGPUBuffer* attrBuf, const scene::E_VERTEX_ATTRIBUTE_ID& attrId, E_FORMAT format, const size_t &stride, size_t offset, uint32_t divisor)
{
    if (attrId>=scene::EVAI_COUNT)
#ifdef _DEBUG
    {
        os::Printer::log("MeshBuffer mapVertexAttrBuffer attribute ID out of range!\n",ELL_ERROR);
        return;
    }
#else
        return;
#endif // _DEBUG

    uint16_t mask = 0x1u<<attrId;
    uint16_t invMask = ~mask;


    size_t newStride;

    if (attrBuf)
    {
        attrBuf->grab();
        newStride = stride!=0u ? stride : getTexelOrBlockSize(format);
        //bind new buffer
        if (mappedAttrBuf[attrId])
            mappedAttrBuf[attrId]->drop();
        else
            individualHashFields.enabledAttribs |= mask;
    }
    else
    {
        if (mappedAttrBuf[attrId])
        {
            individualHashFields.enabledAttribs &= invMask;
            mappedAttrBuf[attrId]->drop();
        }
        format = EF_R32G32B32A32_SFLOAT;
        newStride = 16u;
        offset = 0u;
        divisor = 0u;
    }


    individualHashFields.setAttrFmt(attrId, format);


    const uint32_t maxDivisor = 0x1u<<_IRR_VAO_MAX_ATTRIB_DIVISOR_BITS;
    if (divisor>maxDivisor)
        divisor = maxDivisor;

    if (divisor!=attrDivisor[attrId])
    {
        for (size_t i=0; i<_IRR_VAO_MAX_ATTRIB_DIVISOR_BITS; i++)
        {
            if (divisor&(0x1u<<i))
                individualHashFields.attributeDivisors[i] |= mask; //set
            else
                individualHashFields.attributeDivisors[i] &= invMask; //zero out
        }

        attrDivisor[attrId] = divisor;
    }

    attrFormat[attrId] = format;
    attrStride[attrId] = newStride;
    attrOffset[attrId] = offset;


    mappedAttrBuf[attrId] = attrBuf;
}


}
}


#endif // _IRR_COMPILE_WITH_OPENGL_
