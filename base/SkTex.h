#pragma once
#include "SkBase.h"
#include "stb_image.h"
class SkTex
{
private:
    static const UINT TexturePixelSize = 4;

public:
    unsigned char *data;
    std::vector<UINT8> test_data;
    uint32_t width, height;
    int nrChannels;
    DXGI_FORMAT format;
    D3D12_RESOURCE_DESC textureDesc = {};
    ComPtr<ID3D12Resource> texture;
    ComPtr<ID3D12Resource> textureUploadHeap;
    void Init(std::string path)
    {
        int _width, _height;
        this->data = stbi_load(path.c_str(), &_width, &_height, &nrChannels, 4);
        this->width = static_cast<uint32_t>(_width);
        this->height = static_cast<uint32_t>(_height);
        this->format = DXGI_FORMAT_R8G8B8A8_UNORM;
        if (!data)
        {
            throw std::runtime_error("Failed to load texture in " + path);
        }
    }
    void Setup(SkBase *base)
    {
        textureDesc.MipLevels = 1;
        textureDesc.Format = this->format;
        textureDesc.Width = this->width;
        textureDesc.Height = this->height;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        SK_CHECK(base->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&texture)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

        SK_CHECK(base->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&textureUploadHeap)));

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = data;
        textureData.RowPitch = this->width * TexturePixelSize;
        textureData.SlicePitch = textureData.RowPitch * this->height;
        fprintf(stderr,"%d,%d...\n",this->width,this->height);
        
        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        base->device->CreateShaderResourceView(texture.Get(), &srvDesc, base->srvHeap->GetCPUDescriptorHandleForHeapStart());
        {
            auto cmd=base->BeginCmd();
            UpdateSubresources(cmd.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);

            cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
            
            base->FlushCmd(cmd);
        }
    }
    void InitCheckerboard()
    {
        const UINT TextureWidth = 256;
        const UINT TextureHeight = 256;
        const UINT rowPitch = TextureWidth * TexturePixelSize;
        const UINT cellPitch = rowPitch >> 3;      // The width of a cell in the checkboard texture.
        const UINT cellHeight = TextureWidth >> 3; // The height of a cell in the checkerboard texture.
        const UINT textureSize = rowPitch * TextureHeight;

        // std::vector<UINT8> data(textureSize);
        test_data.resize(textureSize);
        UINT8 *pData = &test_data[0];

        for (UINT n = 0; n < textureSize; n += TexturePixelSize)
        {
            UINT x = n % rowPitch;
            UINT y = n / rowPitch;
            UINT i = x / cellPitch;
            UINT j = y / cellHeight;

            if (i % 2 == j % 2)
            {
                pData[n] = 0x00;     // R
                pData[n + 1] = 0x00; // G
                pData[n + 2] = 0x00; // B
                pData[n + 3] = 0xff; // A
            }
            else
            {
                pData[n] = 0xff;     // R
                pData[n + 1] = 0xff; // G
                pData[n + 2] = 0xff; // B
                pData[n + 3] = 0xff; // A
            }
        }
        this->data=test_data.data();
        this->width=TextureWidth;
        this->height=TextureHeight;
        this->format=DXGI_FORMAT_R8G8B8A8_UNORM;
    }
};
