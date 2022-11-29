//#include "../d3d11/d3d11_options.h"

#include "dxbc_options.h"


namespace dxvk {

    struct D3D11Options {
        D3D11Options(const Config& config, const Rc<DxvkDevice>& device);

        /// Enables speed hack for mapping on deferred contexts
        ///
        /// This can substantially speed up some games, but may
        /// cause issues if the game submits command lists more
        /// than once.
        bool dcSingleUseMode;

        /// Enables workaround to replace NaN render target
        /// outputs with zero
        bool enableRtOutputNanFixup;

        /// Enables out-of-bounds access check for constant
        /// buffers. Workaround for a few broken games that
        /// access random data inside their shaders.
        bool constantBufferRangeCheck;

        /// Zero-initialize workgroup memory
        ///
        /// Workargound for games that don't initialize
        /// TGSM in compute shaders before reading it.
        bool zeroInitWorkgroupMemory;

        /// Force thread-group shared memory barriers
        ///
        /// Workaround for compute shaders that read and
        /// write from the same shared memory location
        /// without explicit synchronization.
        bool forceTgsmBarriers;

        /// Use relaxed memory barriers
        ///
        /// May improve performance in some games,
        /// but might also cause rendering issues.
        bool relaxedBarriers;

        /// Ignore graphics barriers
        ///
        /// May improve performance in some games,
        /// but might also cause rendering issues.
        bool ignoreGraphicsBarriers;

        /// Maximum tessellation factor.
        ///
        /// Limits tessellation factors in tessellation
        /// control shaders. Values from 8 to 64 are
        /// supported, other values will be ignored.
        int32_t maxTessFactor;

        /// Anisotropic filter override
        ///
        /// Enforces anisotropic filtering with the
        /// given anisotropy value for all samplers.
        int32_t samplerAnisotropy;

        /// Declare vertex positions in shaders as invariant
        bool invariantPosition;

        /// Enable float control bits
        bool floatControls;

        /// Back buffer count for the Vulkan swap chain.
        /// Overrides DXGI_SWAP_CHAIN_DESC::BufferCount.
        int32_t numBackBuffers;

        /// Sync interval. Overrides the value
        /// passed to IDXGISwapChain::Present.
        int32_t syncInterval;

        /// Tear-free mode if vsync is disabled
        /// Tearing mode if vsync is enabled
        Tristate tearFree;

        /// Override maximum frame latency if the app specifies
        /// a higher value. May help with frame timing issues.
        int32_t maxFrameLatency;

        /// Limit frame rate
        int32_t maxFrameRate;

        /// Limit discardable resource size
        VkDeviceSize maxImplicitDiscardSize;

        /// Limit size of buffer-mapped images
        VkDeviceSize maxDynamicImageBufferSize;

        /// Defer surface creation until first present call. This
        /// fixes issues with games that create multiple swap chains
        /// for a single window that may interfere with each other.
        bool deferSurfaceCreation;

        /// Forces the sample count of all textures to be 1, and
        /// performs the required shader and resolve fixups.
        bool disableMsaa;

        /// Dynamic resources with the given bind flags will be allocated
        /// in cached system memory. Enabled automatically when recording
        /// an api trace.
        uint32_t cachedDynamicResources;
    };

}



namespace dxvk {
  
  DxbcOptions::DxbcOptions() {

  }


  DxbcOptions::DxbcOptions(const Rc<DxvkDevice>& device, const D3D11Options& options) {
    const Rc<DxvkAdapter> adapter = device->adapter();

    const DxvkDeviceFeatures& devFeatures = device->features();
    const DxvkDeviceInfo& devInfo = adapter->devicePropertiesExt();

    useDepthClipWorkaround
      = !devFeatures.extDepthClipEnable.depthClipEnable;
    useStorageImageReadWithoutFormat
      = devFeatures.core.features.shaderStorageImageReadWithoutFormat;
    useSubgroupOpsForAtomicCounters
      = (devInfo.coreSubgroup.supportedStages     & VK_SHADER_STAGE_COMPUTE_BIT)
     && (devInfo.coreSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT);
    useDemoteToHelperInvocation
      = (devFeatures.extShaderDemoteToHelperInvocation.shaderDemoteToHelperInvocation);
    useSubgroupOpsForEarlyDiscard
      = (devInfo.coreSubgroup.subgroupSize >= 4)
     && (devInfo.coreSubgroup.supportedStages     & VK_SHADER_STAGE_FRAGMENT_BIT)
     && (devInfo.coreSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT);
    useSdivForBufferIndex
      = adapter->matchesDriver(DxvkGpuVendor::Nvidia, VK_DRIVER_ID_NVIDIA_PROPRIETARY_KHR, 0, 0);
    
    switch (device->config().useRawSsbo) {
      case Tristate::Auto:  minSsboAlignment = devInfo.core.properties.limits.minStorageBufferOffsetAlignment; break;
      case Tristate::True:  minSsboAlignment =  4u; break;
      case Tristate::False: minSsboAlignment = ~0u; break;
    }
    
    invariantPosition        = options.invariantPosition;
    enableRtOutputNanFixup   = options.enableRtOutputNanFixup;
    zeroInitWorkgroupMemory  = options.zeroInitWorkgroupMemory;
    forceTgsmBarriers        = options.forceTgsmBarriers;
    disableMsaa              = options.disableMsaa;
    dynamicIndexedConstantBufferAsSsbo = options.constantBufferRangeCheck;

    // Disable subgroup early discard on Nvidia because it may hurt performance
    if (adapter->matchesDriver(DxvkGpuVendor::Nvidia, VK_DRIVER_ID_NVIDIA_PROPRIETARY_KHR, 0, 0))
      useSubgroupOpsForEarlyDiscard = false;
    
    // Figure out float control flags to match D3D11 rules
    if (options.floatControls) {
      if (devInfo.khrShaderFloatControls.shaderSignedZeroInfNanPreserveFloat32)
        floatControl.set(DxbcFloatControlFlag::PreserveNan32);
      if (devInfo.khrShaderFloatControls.shaderSignedZeroInfNanPreserveFloat64)
        floatControl.set(DxbcFloatControlFlag::PreserveNan64);

      if (devInfo.khrShaderFloatControls.denormBehaviorIndependence != VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE) {
        if (devInfo.khrShaderFloatControls.shaderDenormFlushToZeroFloat32)
          floatControl.set(DxbcFloatControlFlag::DenormFlushToZero32);
        if (devInfo.khrShaderFloatControls.shaderDenormPreserveFloat64)
          floatControl.set(DxbcFloatControlFlag::DenormPreserve64);
      }
    }

    if (!devInfo.khrShaderFloatControls.shaderSignedZeroInfNanPreserveFloat32
     || adapter->matchesDriver(DxvkGpuVendor::Amd, VK_DRIVER_ID_MESA_RADV_KHR, 0, VK_MAKE_VERSION(20, 3, 0)))
      enableRtOutputNanFixup = true;
  }
  
}