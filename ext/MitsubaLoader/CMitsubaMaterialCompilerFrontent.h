#ifndef __C_MITSUBA_MATERIAL_COMPILER_FRONTENT_H_INCLUDED__
#define __C_MITSUBA_MATERIAL_COMPILER_FRONTENT_H_INCLUDED__

#include "../../ext/MitsubaLoader/CElementBSDF.h"
#include <irr/asset/material_compiler/IR.h>

namespace irr
{
namespace ext
{
namespace MitsubaLoader
{

class CMitsubaMaterialCompilerFrontent
{
public:
    asset::material_compiler::IR::INode* compileToIRTree(asset::material_compiler::IR* ir, const CElementBSDF* _bsdf);
};

}}}

#endif