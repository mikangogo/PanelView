#include "PvGr_D3D9.hpp"
#include "PvObject.hpp"
#include "PvStringUtils.hpp"

#include <d3d9.h>

#pragma warning(push)
#pragma warning(disable: 26495)
#include <d3dx9.h>
#pragma warning(pop)

#include <list>
#include <ranges>
#include <memory>
#include <vector>
#include <wrl/client.h>


namespace
{
    enum DeviceState
    {
        DeviceState_Invalid = -1,
        DeviceState_Ok = 0,
        DeviceState_Lost,
        DeviceState_Resettable,
    };

    enum ResourcePoolType
    {
        ResourcePoolType_Vram = 0,
        ResourcePoolType_VramDynamic,
        ResourcePoolType_Managed,
        ResourcePoolType_System,
    };

    const std::vector < std::vector < D3DVERTEXELEMENT9>> VertexElements = 
    {
        // PvGrPositionDataType_3D_Color
        {
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, sizeof(float) * 3 + sizeof(float) * 2, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
            D3DDECL_END()
        },

        // PvGrPositionDataType_3D_Uv_Color
        {
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, sizeof(float) * 3, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            {0, sizeof(float) * 3 + sizeof(float) * 2, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
            D3DDECL_END()
        },
    };

    struct PvGrInstance
    {
        Microsoft::WRL::ComPtr<IDirect3D9> D3D9Interface = nullptr;
    };

    struct PvGrContext : PvObject<PvGrContext, PvGrContextHandle>
    {
        D3DCAPS9 D3DCaps = {};
        D3DPRESENT_PARAMETERS D3DPp = {};
        Microsoft::WRL::ComPtr<IDirect3DDevice9> D3D9Device = nullptr;
        DeviceState CurrentDeviceState = DeviceState_Invalid;
        std::list<PvGrTextureHandle> VramTextures;
        std::array<Microsoft::WRL::ComPtr<IDirect3DVertexDeclaration9>, PvGrPositionDataType_MaxLength> D3D9VertexDeclarations;
        Microsoft::WRL::ComPtr<IDirect3DSurface9> D3D9DepthStencilBuffer;
        std::vector<PvGrTransform> ModelTransformStack;

        bool Healthy() const
        {
            return CurrentDeviceState == DeviceState_Ok;
        }

        void RegisterTexture(PvGrTextureHandle texture)
        {
            for (auto& t : VramTextures)
            {
                if (t)
                {
                    continue;
                }

                
                t = texture;
                return;
            }


            VramTextures.push_back(texture);
        }

        void UnregisterTexture(PvGrTextureHandle texture)
        {
            for (auto& t : VramTextures)
            {
                if (t != texture)
                {
                    continue;
                }


                t = nullptr;
                break;
            }
        }

        bool CreateDepthStencil()
        {
            auto hr = D3D9Device->CreateDepthStencilSurface(
                D3DPp.BackBufferWidth, D3DPp.BackBufferHeight,
                D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, D3D9DepthStencilBuffer.GetAddressOf(), nullptr);

            return SUCCEEDED(hr);
        }
    };

    struct PvGrTexture : PvObject<PvGrTexture, PvGrTextureHandle>
    {
        struct NativeTextureParam
        {
            unsigned int Width = 0;
            unsigned int Height = 0;
            unsigned int Levels = 0;
            unsigned long Usage = 0;
            D3DFORMAT Format = D3DFMT_UNKNOWN;
            D3DPOOL Pool = D3DPOOL_DEFAULT;
        };

        void BeforeReset()
        {
            if (PoolType == ResourcePoolType_Vram || PoolType == ResourcePoolType_VramDynamic)
            {
                D3D9Texture.Reset();
            }
        }

        void AfterReset(PvGrContext& context)
        {
            if (PoolType == ResourcePoolType_Vram || PoolType == ResourcePoolType_VramDynamic)
            {
                if (!D3D9TextureSystemMem)
                {
                    return;
                }

                context.D3D9Device->CreateTexture(D3D9TextureDesc.Width, D3D9TextureDesc.Height, 1,
                                                  D3D9TextureDesc.Usage, D3D9TextureDesc.Format, D3D9TextureDesc.Pool,
                                                   D3D9Texture.GetAddressOf(), nullptr);

                if (PoolType == ResourcePoolType_Vram)
                {
                    context.D3D9Device->UpdateTexture(D3D9TextureSystemMem.Get(), D3D9Texture.Get());
                }
            }
        }

        ResourcePoolType PoolType = ResourcePoolType_Vram;
        Microsoft::WRL::ComPtr<IDirect3DTexture9> D3D9Texture = nullptr;
        Microsoft::WRL::ComPtr<IDirect3DTexture9> D3D9TextureSystemMem = nullptr;
        D3DSURFACE_DESC D3D9TextureDesc = {};
        D3DSURFACE_DESC D3D9TextureSystemMemDesc = {};



        PvGrContextHandle ParentContext = nullptr;
    };

    std::unique_ptr<PvGrInstance> Instance;

    bool IsSystemStarted()
    {
        return Instance != nullptr;
    }
}

bool pvGrStartContext(PvGrContextHandle context, const PvGrColor& clearColor)
{
    if (!PvGrContext::Sanity(context))
    {
        return false;
    }

    auto& ctx = PvGrContext::HandleToObject(context);

    if (ctx.CurrentDeviceState == DeviceState_Invalid)
    {
        return false;
    }


    auto hr = ctx.D3D9Device->TestCooperativeLevel();

    if (hr == D3DERR_DEVICELOST)
    {
        ctx.CurrentDeviceState = DeviceState_Lost;
        return false;
    }
    if (hr == D3DERR_DEVICENOTRESET)
    {
        ctx.CurrentDeviceState = DeviceState_Resettable;

        pvGrResetContext(context);

        return false;
    }
    if (hr == D3D_OK)
    {
        ctx.CurrentDeviceState = DeviceState_Ok;
    }
    else
    {
        ctx.CurrentDeviceState = DeviceState_Invalid;
        return false;
    }


    
    hr = ctx.D3D9Device->SetDepthStencilSurface(ctx.D3D9DepthStencilBuffer.Get());

    if (FAILED(hr))
    {
        return false;
    }


    hr = ctx.D3D9Device->BeginScene();

    if (FAILED(hr))
    {
        return false;
    }

    ctx.D3D9Device->Clear(0, nullptr, D3DCLEAR_TARGET, clearColor.ToArgb(), 1.0f, 0);
    ctx.D3D9Device->SetRenderState(D3DRS_LIGHTING, 0);
    ctx.D3D9Device->SetRenderState(D3DRS_ZENABLE, 0);
    ctx.D3D9Device->SetRenderState(D3DRS_ZWRITEENABLE, 0);
    ctx.D3D9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    ctx.D3D9Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    ctx.D3D9Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    ctx.D3D9Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

    ctx.D3D9Device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

    ctx.D3D9Device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    ctx.D3D9Device->SetRenderState(D3DRS_ALPHAREF, 00);
    //ctx.D3D9Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

    ctx.D3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    return true;
}

void pvGrEndContext(PvGrContextHandle context)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }


    auto hr = ctx.D3D9Device->EndScene();

    if (FAILED(hr))
    {
        return;
    }

    hr = ctx.D3D9Device->Present(nullptr, nullptr, nullptr, nullptr);

}

bool pvGrResetContext(PvGrContextHandle context)
{
    if (!PvGrContext::Sanity(context))
    {
        return false;
    }

    auto& ctx = PvGrContext::HandleToObject(context);

    if (ctx.CurrentDeviceState == DeviceState_Invalid)
    {
        return false;
    }


    for (auto& t : ctx.VramTextures)
    {
        auto texture = PvGrTexture::HandleToObject(t);
        texture.BeforeReset();
    }
    ctx.D3D9DepthStencilBuffer.Reset();

    auto hr = ctx.D3D9Device->Reset(&ctx.D3DPp);

    if (FAILED(hr))
    {
        return false;
    }


    if (!ctx.CreateDepthStencil())
    {
        return false;
    }

    for (auto& t : ctx.VramTextures)
    {
        auto texture = PvGrTexture::HandleToObject(t);
        texture.AfterReset(ctx);
    }

    pvPfDebugPrintLine(std::format("pvGrResetContext(): context: {:p}",
        pvStringFmtPointer(context)
    ));

    return true;
}

bool pvGrResetContext(PvGrContextHandle context, const PvWindowBase& window)
{
    if (!PvGrContext::Sanity(context))
    {
        return false;
    }

    auto& ctx = PvGrContext::HandleToObject(context);

    if (ctx.CurrentDeviceState == DeviceState_Invalid)
    {
        return false;
    }

    ctx.D3DPp.BackBufferWidth = window.GetClientRect().Width();
    ctx.D3DPp.BackBufferHeight = window.GetClientRect().Height();

    return pvGrResetContext(context);
}

PvGrContextHandle pvGrCreateContext(const PvWindowBase& window, unsigned int targetAdaptorId)
{
    auto context = std::make_unique<PvGrContext>();

    auto deviceType = D3DDEVTYPE_HAL;

    D3DCAPS9 d3dCaps;
    auto hr = Instance->D3D9Interface->GetDeviceCaps(targetAdaptorId, deviceType, &d3dCaps);

    if (FAILED(hr))
    {
        return nullptr;
    }

    context->D3DCaps = d3dCaps;


    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferWidth = window.GetClientRect().Width();
    d3dpp.BackBufferHeight = window.GetClientRect().Height();
    d3dpp.hDeviceWindow = window.GetNativeHandle();
    d3dpp.EnableAutoDepthStencil = FALSE;
    hr = Instance->D3D9Interface->CreateDevice(targetAdaptorId,
                                                    deviceType,
                                                    window.GetNativeHandle(),
                                                    D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                                                    &d3dpp,
                                                    context->D3D9Device.GetAddressOf()
    );


    if (FAILED(hr))
    {
        return nullptr;
    }

    context->D3DPp = d3dpp;


    if (!context->CreateDepthStencil())
    {
        return nullptr;
    }



    for (auto i = 0; i < PvGrPositionDataType_MaxLength; ++i)
    {
        hr = context->D3D9Device->CreateVertexDeclaration(VertexElements[i].data(),
                                                          context->D3D9VertexDeclarations[i].GetAddressOf());

        if (FAILED(hr))
        {
            return nullptr;
        }
    }

    pvPfDebugPrintLine(std::format("pvGrCreateContext(): instance: {:p}, context: {:p}",
        pvStringFmtPointer(&Instance),
        pvStringFmtPointer(context.get())
    ));

    context->CurrentDeviceState = DeviceState_Ok;
    return context.release();
}

void pvGrDeleteContext(PvGrContextHandle context)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }


    auto ptr = PvGrContext::HandleToPointer(context);

    if (ptr->CurrentDeviceState == DeviceState_Invalid)
    {
        return;
    }


    for (auto& t : ptr->VramTextures)
    {
        if (t)
        {
            pvPfDebugPrintLine(std::format("pvGrDeleteContext(): Unclean Shutdown: instance: {:p}, context: {:p}: VramTextures[n]: {:p}",
                pvStringFmtPointer(&Instance), 
                pvStringFmtPointer(context),
                pvStringFmtPointer(t))
            );
        }
    }
    ptr->VramTextures.clear();

    for (auto& vd : ptr->D3D9VertexDeclarations)
    {
        vd.Reset();
    }


    ptr->D3D9DepthStencilBuffer.Reset();


    ptr->D3D9Device.Reset();

    pvPfDebugPrintLine(std::format("pvGrDeleteContext(): instance: {:p}, context: {:p}",
        pvStringFmtPointer(&Instance),
        pvStringFmtPointer(context)
    ));

    delete ptr;
}

void pvGrInitialize()
{
    if (IsSystemStarted())
    {
        pvPfThrowException("Failed: pvGrInitialize()");
    }


    auto instance = std::make_unique<PvGrInstance>();

    instance->D3D9Interface = Direct3DCreate9(D3D_SDK_VERSION);
    if (!instance->D3D9Interface.Get())
    {
        return;
    }

    pvPfDebugPrintLine(std::format("pvGrInitialize(): [D3D9] Gr Initialized: instance: {:p}",
        pvStringFmtPointer(&instance)
    ));

    Instance = std::move(instance);
}

void pvGrShutdown()
{
    if (!IsSystemStarted())
    {
        pvPfThrowException("Failed: pvGrShutdown()");
    }

    Instance->D3D9Interface.Reset();
    Instance.reset();

    pvPfDebugPrintLine(std::format("pvGrShutdown(): [D3D9] Gr Shutdown: instance: {:p}",
        pvStringFmtPointer(&Instance)
    ));
}

PvGrTextureHandle pvGrGenerateTextureFromFile(PvGrContextHandle context, const PvPfFileInfo& fileInfo, const PvGrColor& colorKey)
{
    PvGrTexture* texture = nullptr;

    do
    {
        if (!PvGrContext::Sanity(context) || fileInfo.GetData().size_bytes() == 0)
        {
            break;
        }


        auto& ctx = PvGrContext::HandleToObject(context);

        if (!ctx.Healthy())
        {
            break;
        }


        Microsoft::WRL::ComPtr<IDirect3DTexture9> d3d9texture;
        auto hr = D3DXCreateTextureFromFileInMemoryEx(
            ctx.D3D9Device.Get(),
            fileInfo.GetData().data(),
            static_cast<UINT>(fileInfo.GetData().size_bytes()),
            D3DX_DEFAULT,
            D3DX_DEFAULT,
            1,
            0,
            D3DFMT_UNKNOWN,
            D3DPOOL_MANAGED,
            D3DX_DEFAULT,
            D3DX_DEFAULT,
            colorKey.ToArgb(),
            nullptr,
            nullptr,
            d3d9texture.GetAddressOf()
        );

        if (FAILED(hr))
        {
            break;
        }

        D3DSURFACE_DESC d3d9TextureDesc;
        hr = d3d9texture->GetLevelDesc(0, &d3d9TextureDesc);

        texture = new PvGrTexture();

        texture->PoolType = ResourcePoolType_Managed;
        texture->D3D9Texture = d3d9texture;
        texture->D3D9TextureDesc = d3d9TextureDesc;
        texture->ParentContext = context;

        ctx.RegisterTexture(texture);
    }
    while (false);

    pvPfDebugPrintLine(std::format("pvGrGenerateTextureFromFile(): Texture Generated: Handle: {:p}, Size: ({:d}, {:d}), FileInfo: {:p}",
        pvStringFmtPointer(texture),
        texture->D3D9TextureDesc.Width, 
        texture->D3D9TextureDesc.Height,
        pvStringFmtPointer(&fileInfo)
    ));

    return texture;
}

void pvGrDeleteTexture(PvGrTextureHandle texture)
{
    if (!PvGrTexture::Sanity(texture))
    {
        return;
    }

    auto ptr = PvGrTexture::HandleToPointer(texture);

    auto& ctx = PvGrContext::HandleToObject(ptr->ParentContext);
    ctx.UnregisterTexture(texture);


    ptr->D3D9Texture.Reset();
    
    pvPfDebugPrintLine(std::format(
        "pvGrDeleteTexture(): Releasing Texture: Handle: {:p}",
        pvStringFmtPointer(texture)
    ));

    delete ptr;
}

void pvGrSetTexture(PvGrContextHandle context, PvGrTextureHandle texture)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }

    auto& device = ctx.D3D9Device;
    auto hr = S_OK;

    if (!texture)
    {
        hr = device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        hr = device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        hr = device->SetTexture(0, nullptr);

        return;
    }

    auto& tex = PvGrTexture::HandleToObject(texture).D3D9Texture;
    

    hr = device->SetTexture(0, tex.Get());

    hr = device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    hr = device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);


    hr = device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    hr = device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    hr = device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    hr = device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    hr = device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    hr = device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    hr = device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    hr = device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void pvGrSetBlendingTexture(PvGrContextHandle context, PvGrTextureHandle texture1, PvGrTextureHandle texture2,
                            double weight)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    if (!PvGrTexture::Sanity(texture1) || !PvGrTexture::Sanity(texture2))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }

    auto& device = ctx.D3D9Device;
    auto& tex1 = PvGrTexture::HandleToObject(texture1).D3D9Texture;
    auto& tex2 = PvGrTexture::HandleToObject(texture1).D3D9Texture;
    auto hr = S_OK;

    hr = device->SetTexture(0, tex1.Get());
    hr = device->SetTexture(1, tex2.Get());

    hr = device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    hr = device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    hr = device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    hr = device->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    hr = device->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    hr = device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    hr = device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    hr = device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
    hr = device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    hr = device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    hr = device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CONSTANT);
    hr = device->SetTextureStageState(1, D3DTSS_CONSTANT, D3DCOLOR_ARGB(static_cast<int>(weight * 255.0), 0, 0, 0));

    hr = device->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
    hr = device->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void pvGrGetTextureSize(PvGrTextureHandle texture, PvGrSize2D& size)
{
    if (!PvGrTexture::Sanity(texture))
    {
        return;
    }

    const auto& tex = PvGrTexture::HandleToObject(texture);

    size.Width = tex.D3D9TextureDesc.Width;
    size.Height = tex.D3D9TextureDesc.Height;
}

void pvGrSetProjectionTransform(PvGrContextHandle context, const PvGrTransform& transform)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }


    D3DMATRIX m;
    transform.ToFloatArray(m.m[0]);

    ctx.D3D9Device->SetTransform(D3DTS_PROJECTION, &m);
}

void pvGrSetViewTransform(PvGrContextHandle context, const PvGrTransform& transform)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }


    D3DMATRIX m;
    transform.ToFloatArray(m.m[0]);

    ctx.D3D9Device->SetTransform(D3DTS_VIEW, &m);
}

void pvGrSetModelTransform(PvGrContextHandle context, const PvGrTransform& transform)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }


    D3DMATRIX m;
    transform.ToFloatArray(m.m[0]);

    ctx.D3D9Device->SetTransform(D3DTS_WORLDMATRIX(0), &m);
}

void pvGrApplyModelTransformStack(PvGrContextHandle context)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }

    auto m0 = PvGrTransform::Identity();

    for (auto& mn : std::ranges::reverse_view(ctx.ModelTransformStack))
    {
        m0 *= mn;
    }
    
    D3DMATRIX m;
    m0.ToFloatArray(m.m[0]);

    ctx.D3D9Device->SetTransform(D3DTS_WORLDMATRIX(0), &m);
}

void pvGrClearModelTransformStack(PvGrContextHandle context)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    auto& ctx = PvGrContext::HandleToObject(context);

    ctx.ModelTransformStack.clear();
}

void pvGrPushModelTransformStack(PvGrContextHandle context, const PvGrTransform& transform)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    auto& ctx = PvGrContext::HandleToObject(context);

    ctx.ModelTransformStack.push_back(transform);
}

void pvGrPopModelTransformStack(PvGrContextHandle context)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    auto& ctx = PvGrContext::HandleToObject(context);

    ctx.ModelTransformStack.pop_back();
}

void pvGrSetViewport(PvGrContextHandle context, const PvGrViewPort& viewport)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }

    D3DVIEWPORT9 vp = {
        .X = static_cast<DWORD>(viewport.X),
        .Y = static_cast<DWORD>(viewport.Y),
        .Width = static_cast<DWORD>(viewport.Width),
        .Height = static_cast<DWORD>(viewport.Height),
        .MinZ = static_cast<float>(viewport.MinimumZ),
        .MaxZ = static_cast<float>(viewport.MaximumZ)
    };
    auto hr = ctx.D3D9Device->SetViewport(&vp);
}

void pvGrGetViewport(PvGrContextHandle context, PvGrViewPort& viewport)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }

    D3DVIEWPORT9 vp = {};
    auto hr = ctx.D3D9Device->GetViewport(&vp);

    viewport = PvGrViewPort(vp.X, vp.Y, vp.Width, vp.Height, vp.MinZ, vp.MaxZ);
}

void pvGrDrawTriangles(PvGrContextHandle context, const std::span<const PvGrPositionData_3D>& vertices)
{
    if (!PvGrContext::Sanity(context))
    {
        return;
    }

    const auto& ctx = PvGrContext::HandleToObject(context);

    if (!ctx.Healthy())
    {
        return;
    }

    HRESULT hr;
    const auto& device = ctx.D3D9Device;

    
    const unsigned int faces = static_cast<unsigned int>(vertices.size() / 3);
    constexpr unsigned int stride = sizeof(PvGrPositionData_3D);


    Microsoft::WRL::ComPtr<IDirect3DBaseTexture9> previousTexture;
    hr = device->GetTexture(0, previousTexture.GetAddressOf());

    if (previousTexture.Get())
    {
        hr = device->SetVertexDeclaration(ctx.D3D9VertexDeclarations[PvGrPositionDataType_3D_Uv_Color].Get());
    }
    else
    {
        hr = device->SetVertexDeclaration(ctx.D3D9VertexDeclarations[PvGrPositionDataType_3D_Color].Get());
    }


    hr = device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, faces, vertices.data(), stride);
}

void pvGrDrawRectangle(PvGrContextHandle context, const double width, const double height, const PvGrColor& color)
{
    const auto x2 = width, x1 = 0.0;
    const auto y1 = 0.0, y2 = (height);
    constexpr auto u1 = 0.0, u2 = 1.0;
    constexpr auto v1 = 0.0, v2 = 1.0;

    const std::array positions = 
    {
        PvGrVector3D({.X = x1, .Y = y1, .Z = 0.0}),
        PvGrVector3D({.X = x2, .Y = y2, .Z = 0.0}),
        PvGrVector3D({.X = x1, .Y = y2, .Z = 0.0}),
        PvGrVector3D({.X = x1, .Y = y1, .Z = 0.0}),
        PvGrVector3D({.X = x2, .Y = y1, .Z = 0.0}),
        PvGrVector3D({.X = x2, .Y = y2, .Z = 0.0})
    };
    constexpr std::array uvs = 
    {
        PvGrVector2D({.X = u1, .Y = v1}),
        PvGrVector2D({.X = u2, .Y = v2}),
        PvGrVector2D({.X = u1, .Y = v2}),
        PvGrVector2D({.X = u1, .Y = v1}),
        PvGrVector2D({.X = u2, .Y = v1}),
        PvGrVector2D({.X = u2, .Y = v2}),
    };

    const std::array vertices = 
    {
        PvGrPositionData_3D(positions[0], uvs[0], color),
        PvGrPositionData_3D(positions[1], uvs[1], color),
        PvGrPositionData_3D(positions[2], uvs[2], color),
        PvGrPositionData_3D(positions[3], uvs[3], color),
        PvGrPositionData_3D(positions[4], uvs[4], color),
        PvGrPositionData_3D(positions[5], uvs[5], color),
    };

    pvGrDrawTriangles(context, vertices);
}
