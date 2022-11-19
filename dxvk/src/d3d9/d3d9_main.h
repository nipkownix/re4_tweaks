#pragma once
#include "../dxvk/dxvk_instance.h"

#include "d3d9_interface.h"
#include "d3d9_shader_validator.h"

#include "d3d9_annotation.h"


namespace dxvk {

    HRESULT CreateD3D9(bool Extended, IDirect3D9Ex** ppDirect3D9Ex);
}
