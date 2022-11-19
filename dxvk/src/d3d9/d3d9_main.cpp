#include "d3d9_main.h"

class D3DFE_PROCESSVERTICES;
using PSGPERRORID = UINT;

namespace dxvk {
  Logger Logger::s_instance("");
  D3D9GlobalAnnotationList D3D9GlobalAnnotationList::s_instance;

  HRESULT CreateD3D9(
          bool           Extended,
          IDirect3D9Ex** ppDirect3D9Ex) {
    if (!ppDirect3D9Ex)
      return D3DERR_INVALIDCALL;

    *ppDirect3D9Ex = ref(new D3D9InterfaceEx( Extended ));
    return D3D_OK;
  }
}