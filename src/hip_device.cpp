/*
Copyright (c) 2015 - present Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "hip/hip_runtime.h"
#include "hip_hcc_internal.h"
#include "trace_helper.h"
#include "device_util.h"

//-------------------------------------------------------------------------------------------------
// Devices
//-------------------------------------------------------------------------------------------------
// TODO - does this initialize HIP runtime?
hipError_t hipGetDevice(int* deviceId) {
    HIP_INIT_API(hipGetDevice, deviceId);

    hipError_t e = hipSuccess;
    if (deviceId == nullptr)
        return ihipLogStatus(hipErrorInvalidValue);

    auto ctx = ihipGetTlsDefaultCtx();

    if (ctx == nullptr) {
        e = hipErrorInvalidDevice;  // TODO, check error code.
        *deviceId = -1;
    } else {
        *deviceId = ctx->getDevice()->_deviceId;
    }

    return ihipLogStatus(e);
}

// TODO - does this initialize HIP runtime?
hipError_t ihipGetDeviceCount(int* count) {
    hipError_t e = hipSuccess;

    if (count != nullptr) {
        *count = g_deviceCnt;

        if (*count > 0) {
            e = hipSuccess;
        } else {
            e = hipErrorNoDevice;
        }
    } else {
        e = hipErrorInvalidValue;
    }
    return e;
}

hipError_t hipGetDeviceCount(int* count) {
    HIP_INIT_API(hipGetDeviceCount, count);
    return ihipLogStatus(ihipGetDeviceCount(count));
}

hipError_t hipDeviceSetCacheConfig(hipFuncCache_t cacheConfig) {
    HIP_INIT_API(hipDeviceSetCacheConfig, cacheConfig);

    // Nop, AMD does not support variable cache configs.

    return ihipLogStatus(hipSuccess);
}

hipError_t hipDeviceGetCacheConfig(hipFuncCache_t* cacheConfig) {
    HIP_INIT_API(hipDeviceGetCacheConfig, cacheConfig);

    if (cacheConfig == nullptr) {
        return ihipLogStatus(hipErrorInvalidValue);
    }

    *cacheConfig = hipFuncCachePreferNone;

    return ihipLogStatus(hipSuccess);
}

hipError_t hipDeviceGetLimit(size_t* pValue, hipLimit_t limit) {
    HIP_INIT_API(hipDeviceGetLimit, pValue, limit);
    if (pValue == nullptr) {
        return ihipLogStatus(hipErrorInvalidValue);
    }
    if (limit == hipLimitMallocHeapSize) {
        *pValue = (size_t)__HIP_SIZE_OF_HEAP;
        return ihipLogStatus(hipSuccess);
    } else {
        return ihipLogStatus(hipErrorUnsupportedLimit);
    }
}

hipError_t hipFuncSetCacheConfig(const void* func, hipFuncCache_t cacheConfig) {
    HIP_INIT_API(hipFuncSetCacheConfig, cacheConfig);

    // Nop, AMD does not support variable cache configs.

    return ihipLogStatus(hipSuccess);
}

hipError_t hipDeviceSetSharedMemConfig(hipSharedMemConfig config) {
    HIP_INIT_API(hipDeviceSetSharedMemConfig, config);

    // Nop, AMD does not support variable shared mem configs.

    return ihipLogStatus(hipSuccess);
}

hipError_t hipDeviceGetSharedMemConfig(hipSharedMemConfig* pConfig) {
    HIP_INIT_API(hipDeviceGetSharedMemConfig, pConfig);

    *pConfig = hipSharedMemBankSizeFourByte;

    return ihipLogStatus(hipSuccess);
}

hipError_t hipSetDevice(int deviceId) {
    HIP_INIT_API(hipSetDevice, deviceId);
    if ((deviceId < 0) || (deviceId >= g_deviceCnt)) {
        return ihipLogStatus(hipErrorInvalidDevice);
    } else {
        ihipSetTlsDefaultCtx(ihipGetPrimaryCtx(deviceId));
        tls->getPrimaryCtx = true;
        return ihipLogStatus(hipSuccess);
    }
}

hipError_t hipDeviceSynchronize(void) {
    HIP_INIT_SPECIAL_API(hipDeviceSynchronize, TRACE_SYNC);
    return ihipLogStatus(ihipSynchronize(tls));
}

hipError_t hipDeviceReset(void) {
    HIP_INIT_API(hipDeviceReset, );

    auto* ctx = ihipGetTlsDefaultCtx();

    // TODO-HCC
    // This function currently does a user-level cleanup of known resources.
    // It could benefit from KFD support to perform a more "nuclear" clean that would include any
    // associated kernel resources and page table entries.

#if 0
    if (ctx) {
        // Release ctx resources (streams and memory):
        ctx->locked_reset();
    }
#endif
    if (ctx) {
        ihipDevice_t* deviceHandle = ctx->getWriteableDevice();
        deviceHandle->locked_reset();
    }

    return ihipLogStatus(hipSuccess);
}


hipError_t ihipDeviceSetState(TlsData *tls) {
    hipError_t e = hipErrorInvalidContext;
    auto* ctx = ihipGetTlsDefaultCtx();

    if (ctx) {
        ihipDevice_t* deviceHandle = ctx->getWriteableDevice();
        if (deviceHandle->_state == 0) {
            deviceHandle->_state = 1;
        }
        e = hipSuccess;
    }

    return e;
}


hipError_t ihipDeviceGetAttribute(int* pi, hipDeviceAttribute_t attr, int device) {
    hipError_t e = hipSuccess;

    if (pi == nullptr) {
        return hipErrorInvalidValue;
    }

    auto* hipDevice = ihipGetDevice(device);
    hipDeviceProp_t* prop = &hipDevice->_props;
    if (hipDevice) {
        switch (attr) {
            case hipDeviceAttributeMaxThreadsPerBlock:
                *pi = prop->maxThreadsPerBlock;
                break;
            case hipDeviceAttributeMaxBlockDimX:
                *pi = prop->maxThreadsDim[0];
                break;
            case hipDeviceAttributeMaxBlockDimY:
                *pi = prop->maxThreadsDim[1];
                break;
            case hipDeviceAttributeMaxBlockDimZ:
                *pi = prop->maxThreadsDim[2];
                break;
            case hipDeviceAttributeMaxGridDimX:
                *pi = prop->maxGridSize[0];
                break;
            case hipDeviceAttributeMaxGridDimY:
                *pi = prop->maxGridSize[1];
                break;
            case hipDeviceAttributeMaxGridDimZ:
                *pi = prop->maxGridSize[2];
                break;
            case hipDeviceAttributeMaxSharedMemoryPerBlock:
                *pi = prop->sharedMemPerBlock;
                break;
            case hipDeviceAttributeTotalConstantMemory:
                *pi = prop->totalConstMem;
                break;
            case hipDeviceAttributeWarpSize:
                *pi = prop->warpSize;
                break;
            case hipDeviceAttributeMaxRegistersPerBlock:
                *pi = prop->regsPerBlock;
                break;
            case hipDeviceAttributeClockRate:
                *pi = prop->clockRate;
                break;
            case hipDeviceAttributeMemoryClockRate:
                *pi = prop->memoryClockRate;
                break;
            case hipDeviceAttributeMemoryBusWidth:
                *pi = prop->memoryBusWidth;
                break;
            case hipDeviceAttributeMultiprocessorCount:
                *pi = prop->multiProcessorCount;
                break;
            case hipDeviceAttributeComputeMode:
                *pi = prop->computeMode;
                break;
            case hipDeviceAttributeL2CacheSize:
                *pi = prop->l2CacheSize;
                break;
            case hipDeviceAttributeMaxThreadsPerMultiProcessor:
                *pi = prop->maxThreadsPerMultiProcessor;
                break;
            case hipDeviceAttributeComputeCapabilityMajor:
                *pi = prop->major;
                break;
            case hipDeviceAttributeComputeCapabilityMinor:
                *pi = prop->minor;
                break;
            case hipDeviceAttributePciBusId:
                *pi = prop->pciBusID;
                break;
            case hipDeviceAttributeConcurrentKernels:
                *pi = prop->concurrentKernels;
                break;
            case hipDeviceAttributePciDeviceId:
                *pi = prop->pciDeviceID;
                break;
            case hipDeviceAttributeMaxSharedMemoryPerMultiprocessor:
                *pi = prop->maxSharedMemoryPerMultiProcessor;
                break;
            case hipDeviceAttributeIsMultiGpuBoard:
                *pi = prop->isMultiGpuBoard;
                break;
            case hipDeviceAttributeIntegrated:
                *pi = prop->integrated;
                break;
            case hipDeviceAttributeMaxTexture1DWidth:
                *pi = prop->maxTexture1D;
                break;
            case hipDeviceAttributeMaxTexture2DWidth:
                *pi = prop->maxTexture2D[0];
                break;
            case hipDeviceAttributeMaxTexture2DHeight:
                *pi = prop->maxTexture2D[1];
                break;
            case hipDeviceAttributeMaxTexture3DWidth:
                *pi = prop->maxTexture3D[0];
                break;
            case hipDeviceAttributeMaxTexture3DHeight:
                *pi = prop->maxTexture3D[1];
                break;
            case hipDeviceAttributeMaxTexture3DDepth:
                *pi = prop->maxTexture3D[2];
                break;
            case hipDeviceAttributeHdpMemFlushCntl:
                {
                    uint32_t** hdp = reinterpret_cast<uint32_t**>(pi);
                    *hdp = prop->hdpMemFlushCntl;
                }
                break;
            case hipDeviceAttributeHdpRegFlushCntl:
                {
                    uint32_t** hdp = reinterpret_cast<uint32_t**>(pi);
                    *hdp = prop->hdpRegFlushCntl;
                }
                break;
            case hipDeviceAttributeCooperativeLaunch:
                *pi = prop->cooperativeLaunch;
                break;
            case hipDeviceAttributeCooperativeMultiDeviceLaunch:
                *pi = prop->cooperativeMultiDeviceLaunch;
                break;
            case hipDeviceAttributeCooperativeMultiDeviceUnmatchedFunc:
                *pi = prop->cooperativeMultiDeviceUnmatchedFunc;
                break;
            case hipDeviceAttributeCooperativeMultiDeviceUnmatchedGridDim:
                *pi = prop->cooperativeMultiDeviceUnmatchedGridDim;
                break;
            case hipDeviceAttributeCooperativeMultiDeviceUnmatchedBlockDim:
                *pi = prop->cooperativeMultiDeviceUnmatchedBlockDim;
                break;
            case hipDeviceAttributeCooperativeMultiDeviceUnmatchedSharedMem:
                *pi = prop->cooperativeMultiDeviceUnmatchedSharedMem;
                break;
            case hipDeviceAttributeMaxPitch:
                *pi = prop->memPitch;
                break;
            case hipDeviceAttributeTextureAlignment:
                *pi = prop->textureAlignment;
                break;
            case hipDeviceAttributeTexturePitchAlignment:
                *pi = prop->texturePitchAlignment;
                 break;
            case hipDeviceAttributeKernelExecTimeout:
                *pi = prop->kernelExecTimeoutEnabled;
                break;
            case hipDeviceAttributeCanMapHostMemory:
                *pi = prop->canMapHostMemory;
                break;
            case hipDeviceAttributeEccEnabled:
                *pi = prop->ECCEnabled;
                break;
            default:
                e = hipErrorInvalidValue;
                break;
        }
    } else {
        e = hipErrorInvalidDevice;
    }
    return e;
}

hipError_t hipDeviceGetAttribute(int* pi, hipDeviceAttribute_t attr, int device) {
    HIP_INIT_API(hipDeviceGetAttribute, pi, attr, device);
    if ((device < 0) || (device >= g_deviceCnt)) {
        return ihipLogStatus(hipErrorInvalidDevice);
    }
    return ihipLogStatus(ihipDeviceGetAttribute(pi, attr, device));
}

hipError_t ihipGetDeviceProperties(hipDeviceProp_t* props, int device) {
    hipError_t e;

    if (props != nullptr) {
        auto* hipDevice = ihipGetDevice(device);
        if (hipDevice) {
            // copy saved props
            *props = hipDevice->_props;
            e = hipSuccess;
        } else {
            e = hipErrorInvalidDevice;
        }
    } else {
        e = hipErrorInvalidDevice;
    }

    return e;
}

hipError_t hipGetDeviceProperties(hipDeviceProp_t* props, int device) {
    HIP_INIT_API(hipGetDeviceProperties, props, device);
    if ((device < 0) || (device >= g_deviceCnt)) {
        return ihipLogStatus(hipErrorInvalidDevice);
    }
    return ihipLogStatus(ihipGetDeviceProperties(props, device));
}

hipError_t hipSetDeviceFlags(unsigned int flags) {
    HIP_INIT_API(hipSetDeviceFlags, flags);

    hipError_t e = hipSuccess;

    auto* ctx = ihipGetTlsDefaultCtx();

    // TODO : does this really OR in the flags or replaces previous flags:
    // TODO : Review error handling behavior for this function, it often returns
    // ErrorSetOnActiveProcess
    if (ctx) {
        auto* deviceHandle = ctx->getDevice();
        if (deviceHandle->_state == 0) {
            ctx->_ctxFlags = ctx->_ctxFlags | flags;
            if (flags & hipDeviceScheduleMask) {
                switch (hipDeviceScheduleMask) {
                    case hipDeviceScheduleAuto:
                    case hipDeviceScheduleSpin:
                    case hipDeviceScheduleYield:
                    case hipDeviceScheduleBlockingSync:
                        e = hipSuccess;
                        break;
                    default:
                        e = hipSuccess;  // TODO - should this be error?  Map to Auto?
                        // e = hipErrorInvalidValue;
                        break;
                }
            }

            unsigned supportedFlags =
                hipDeviceScheduleMask | hipDeviceMapHost | hipDeviceLmemResizeToMax;

            if (flags & (~supportedFlags)) {
                e = hipErrorInvalidValue;
            }
        } else {
            e = hipErrorSetOnActiveProcess;
        }
    } else {
        e = hipErrorInvalidDevice;
    }

    return ihipLogStatus(e);
};

hipError_t hipDeviceComputeCapability(int* major, int* minor, hipDevice_t device) {
    HIP_INIT_API(hipDeviceComputeCapability, major, minor, device);
    hipError_t e = hipSuccess;
    if ((device < 0) || (device >= g_deviceCnt)) {
        e = hipErrorInvalidDevice;
    } else {
        e = ihipDeviceGetAttribute(major, hipDeviceAttributeComputeCapabilityMajor, device);
        e = ihipDeviceGetAttribute(minor, hipDeviceAttributeComputeCapabilityMinor, device);
    }
    return ihipLogStatus(e);
}

hipError_t hipDeviceGetName(char* name, int len, hipDevice_t device) {
    // Cast to void* here to avoid printing garbage in debug modes.
    HIP_INIT_API(hipDeviceGetName, (void*)name, len, device);
    hipError_t e = hipSuccess;
    if ((device < 0) || (device >= g_deviceCnt)) {
        e = hipErrorInvalidDevice;
    } else {
        auto deviceHandle = ihipGetDevice(device);
        int nameLen = strlen(deviceHandle->_props.name);
        if (nameLen <= len) memcpy(name, deviceHandle->_props.name, nameLen);
    }
    return ihipLogStatus(e);
}

hipError_t hipDeviceGetPCIBusId(char* pciBusId, int len, int device) {
    // Cast to void* here to avoid printing garbage in debug modes.
    HIP_INIT_API(hipDeviceGetPCIBusId, (void*)pciBusId, len, device);
    hipError_t e = hipErrorInvalidValue;
    if ((device < 0) || (device >= g_deviceCnt)) {
        e = hipErrorInvalidDevice;
    } else {
        if ((pciBusId != nullptr) && (len > 0)) {
            auto deviceHandle = ihipGetDevice(device);
            int retVal =
                snprintf(pciBusId, len, "%04x:%02x:%02x.0", deviceHandle->_props.pciDomainID,
                         deviceHandle->_props.pciBusID, deviceHandle->_props.pciDeviceID);
            if (retVal > 0 && retVal < len) {
                e = hipSuccess;
            }
        }
    }
    return ihipLogStatus(e);
}

hipError_t hipDeviceTotalMem(size_t* bytes, hipDevice_t device) {
    HIP_INIT_API(hipDeviceTotalMem, bytes, device);
    hipError_t e = hipSuccess;
    if ((device < 0) || (device >= g_deviceCnt)) {
        e = hipErrorInvalidDevice;
    } else {
        auto deviceHandle = ihipGetDevice(device);
        *bytes = deviceHandle->_props.totalGlobalMem;
    }
    return ihipLogStatus(e);
}

hipError_t hipDeviceGetByPCIBusId(int* device, const char* pciBusId) {
    HIP_INIT_API(hipDeviceGetByPCIBusId, device, pciBusId);
    hipDeviceProp_t tempProp;
    int deviceCount = 0;
    hipError_t e = hipErrorInvalidValue;
    if ((device != nullptr) && (pciBusId != nullptr)) {
        int pciBusID = -1;
        int pciDeviceID = -1;
        int pciDomainID = -1;
        int len = 0;
        len = sscanf(pciBusId, "%04x:%02x:%02x", &pciDomainID, &pciBusID, &pciDeviceID);
        if (len == 3) {
            ihipGetDeviceCount(&deviceCount);
            for (int i = 0; i < deviceCount; i++) {
                ihipGetDeviceProperties(&tempProp, i);
                if (tempProp.pciBusID == pciBusID) {
                    *device = i;
                    e = hipSuccess;
                    break;
                }
            }
        }
    }
    return ihipLogStatus(e);
}

hipError_t hipChooseDevice(int* device, const hipDeviceProp_t* prop) {
    HIP_INIT_API(hipChooseDevice, device, prop);
    hipDeviceProp_t tempProp;
    hipError_t e = hipSuccess;
    if ((device == NULL) || (prop == NULL)) {
        e = hipErrorInvalidValue;
    }
    if (e == hipSuccess) {
        int deviceCount;
        int inPropCount = 0;
        int matchedPropCount = 0;
        ihipGetDeviceCount(&deviceCount);
        *device = 0;
        for (int i = 0; i < deviceCount; i++) {
            ihipGetDeviceProperties(&tempProp, i);
            if (prop->major != 0) {
                inPropCount++;
                if (tempProp.major >= prop->major) {
                    matchedPropCount++;
                }
                if (prop->minor != 0) {
                    inPropCount++;
                    if (tempProp.minor >= prop->minor) {
                        matchedPropCount++;
                    }
                }
            }
            if (prop->totalGlobalMem != 0) {
                inPropCount++;
                if (tempProp.totalGlobalMem >= prop->totalGlobalMem) {
                    matchedPropCount++;
                }
            }
            if (prop->sharedMemPerBlock != 0) {
                inPropCount++;
                if (tempProp.sharedMemPerBlock >= prop->sharedMemPerBlock) {
                    matchedPropCount++;
                }
            }
            if (prop->maxThreadsPerBlock != 0) {
                inPropCount++;
                if (tempProp.maxThreadsPerBlock >= prop->maxThreadsPerBlock) {
                    matchedPropCount++;
                }
            }
            if (prop->totalConstMem != 0) {
                inPropCount++;
                if (tempProp.totalConstMem >= prop->totalConstMem) {
                    matchedPropCount++;
                }
            }
            if (prop->multiProcessorCount != 0) {
                inPropCount++;
                if (tempProp.multiProcessorCount >= prop->multiProcessorCount) {
                    matchedPropCount++;
                }
            }
            if (prop->maxThreadsPerMultiProcessor != 0) {
                inPropCount++;
                if (tempProp.maxThreadsPerMultiProcessor >= prop->maxThreadsPerMultiProcessor) {
                    matchedPropCount++;
                }
            }
            if (prop->memoryClockRate != 0) {
                inPropCount++;
                if (tempProp.memoryClockRate >= prop->memoryClockRate) {
                    matchedPropCount++;
                }
            }
            if (inPropCount == matchedPropCount) {
                *device = i;
            }
#if 0
        else{
            e= hipErrorInvalidValue;
        }
#endif
        }
    }
    return ihipLogStatus(e);
}

#define HSA_ERROR_CHECK(hsa_error_code) \
    if ((hsa_error_code != HSA_STATUS_SUCCESS) && (hsa_error_code != HSA_STATUS_INFO_BREAK)) { \
        return ihipLogStatus(hipErrorRuntimeOther); \
    }

hipError_t hipExtGetLinkTypeAndHopCount(int device1, int device2, uint32_t* linktype, uint32_t* hopcount) {
    HIP_INIT_API(hipExtGetLinkTypeAndHopCount, device1, device2, linktype, hopcount);

    if ((device1 < 0) || (device1 >= g_deviceCnt) || (device2 < 0) || (device2 >= g_deviceCnt)) {
        return ihipLogStatus(hipErrorInvalidDevice);
    } else {
        auto device1Handle = ihipGetDevice(device1);
        auto device2Handle = ihipGetDevice(device2);

        const auto& find_pool = [](hsa_amd_memory_pool_t pool, void* data) {
            bool allowed;
            hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALLOWED, &allowed);
            if (allowed) {
                hsa_amd_segment_t segment;
                hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_SEGMENT, &segment);
                if (HSA_AMD_SEGMENT_GLOBAL != segment) return HSA_STATUS_SUCCESS;

                uint32_t flags;
                hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_GLOBAL_FLAGS, &flags);
                if (flags & HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_COARSE_GRAINED) {
                    *((hsa_amd_memory_pool_t*)data) = pool;
                    return HSA_STATUS_INFO_BREAK;
                }
            }
            return HSA_STATUS_SUCCESS;
        };

        hsa_status_t err;
        hsa_amd_memory_pool_t pool;
        err = hsa_amd_agent_iterate_memory_pools(device2Handle->_hsaAgent, find_pool, (void*)&pool);
        HSA_ERROR_CHECK(err);

        hsa_amd_memory_pool_link_info_t link_info;
        err = hsa_amd_agent_memory_pool_get_info(device1Handle->_hsaAgent, pool, HSA_AMD_AGENT_MEMORY_POOL_INFO_LINK_INFO, &link_info);
        HSA_ERROR_CHECK(err);
        *linktype = link_info.link_type;

        if (link_info.numa_distance < 30)
            *hopcount = 1;
        else
            *hopcount = 2;
        return ihipLogStatus(hipSuccess);
    }
}
