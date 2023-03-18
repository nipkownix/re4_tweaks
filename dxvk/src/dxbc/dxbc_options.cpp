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

        /// Zero-initialize workgroup memory
        ///
        /// Workargound for games that don't initialize
        /// TGSM in compute shaders before reading it.
        bool zeroInitWorkgroupMemory;

        /// Force thread-group shared memory accesses to be volatile
        ///
        /// Workaround for compute shaders that read and
        /// write from the same shared memory location
        /// without explicit synchronization.
        bool forceVolatileTgsmAccess;

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

        /// Mipmap LOD bias
        ///
        /// Enforces the given LOD bias for all samplers.
        float samplerLodBias;

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

        /// Enables sample rate shading by interpolating fragment shader
        /// inputs at the sample location rather than pixel center,
        /// unless otherwise specified by the application.
        bool forceSampleRateShading;

        /// Forces the sample count of all textures to be 1, and
        /// performs the required shader and resolve fixups.
        bool disableMsaa;

        /// Dynamic resources with the given bind flags will be allocated
        /// in cached system memory. Enabled automatically when recording
        /// an api trace.
        uint32_t cachedDynamicResources;

        /// Always lock immediate context on every API call. May be
        /// useful for debugging purposes or when applications have
        /// race conditions.
        bool enableContextLock;

        /// Shader dump path
        std::string shaderDumpPath;
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
    useSubgroupOpsForAtomicCounters
      = (devInfo.vk11.subgroupSupportedStages     & VK_SHADER_STAGE_COMPUTE_BIT)
     && (devInfo.vk11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT);

    VkFormatFeatureFlags2 r32Features
      = device->getFormatFeatures(VK_FORMAT_R32_SFLOAT).optimal
      & device->getFormatFeatures(VK_FORMAT_R32_UINT).optimal
      & device->getFormatFeatures(VK_FORMAT_R32_SINT).optimal;

    supportsTypedUavLoadR32 = (r32Features & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT);

    switch (device->config().useRawSsbo) {
      case Tristate::Auto:  minSsboAlignment = devInfo.core.properties.limits.minStorageBufferOffsetAlignment; break;
      case Tristate::True:  minSsboAlignment =  4u; break;
      case Tristate::False: minSsboAlignment = ~0u; break;
    }
    
    invariantPosition        = options.invariantPosition;
    zeroInitWorkgroupMemory  = options.zeroInitWorkgroupMemory;
    forceVolatileTgsmAccess  = options.forceVolatileTgsmAccess;
    disableMsaa              = options.disableMsaa;
    forceSampleRateShading   = options.forceSampleRateShading;
    enableSampleShadingInterlock = device->features().extFragmentShaderInterlock.fragmentShaderSampleInterlock;

    // Figure out float control flags to match D3D11 rules
    if (options.floatControls) {
      if (devInfo.vk12.shaderSignedZeroInfNanPreserveFloat32)
        floatControl.set(DxbcFloatControlFlag::PreserveNan32);
      if (devInfo.vk12.shaderSignedZeroInfNanPreserveFloat64)
        floatControl.set(DxbcFloatControlFlag::PreserveNan64);

      if (devInfo.vk12.denormBehaviorIndependence != VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE) {
        if (devInfo.vk12.shaderDenormFlushToZeroFloat32)
          floatControl.set(DxbcFloatControlFlag::DenormFlushToZero32);
        if (devInfo.vk12.shaderDenormPreserveFloat64)
          floatControl.set(DxbcFloatControlFlag::DenormPreserve64);
      }
    }
  }
  
}