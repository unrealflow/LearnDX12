#pragma once
#include "SkBase.h"
#include "SkAgent.h"
#include "stb_image.h"
class SkTex
{
private:
    SkBase *base;
    SkAgent *agent;

    void Filter(std::vector<unsigned char> &input,
                std::vector<unsigned char> &output,
                uint32_t &width, uint32_t &height)
    {
        int ksize = 2;
        uint32_t f_width = width / ksize;
        uint32_t f_height = height / ksize;
        output.resize(f_width * f_height * Channels);
        for (uint32_t i = 0; i < f_height; i++)
        {
            for (uint32_t j = 0; j < f_width; j++)
            {
                uint32_t pixel[4] = {0, 0, 0, 1};
                uint32_t index0 = Channels * (i * f_width + j);
                uint32_t index1 = Channels * ksize * (i * width + j);
                for (uint32_t c = 0; c < Channels; c++)
                {
                    pixel[c] += input[index1 + c];
                    pixel[c] += input[index1 + Channels + c];
                    pixel[c] += input[Channels * width + index1 + c];
                    pixel[c] += input[Channels * width + index1 + Channels + c];
                    pixel[c] = pixel[c] / 4;
                    output[index0 + c] = (unsigned char)pixel[c];
                }
            }
        }
        width = f_width;
        height = f_height;
    }

public:
    static const UINT Channels = 4;
    std::vector<unsigned char> pixelData;
    std::vector<std::vector<unsigned char>> mip;
    uint32_t width, height;
    DXGI_FORMAT format;
    uint32_t mipLevels = 1;
    D3D12_RESOURCE_DESC textureDesc = {};
    ComPtr<ID3D12Resource> texture;
    ComPtr<ID3D12Resource> textureUploadHeap;
    D3D12_PACKED_MIP_INFO packedMipInfo = {};
    std::vector<ComPtr<ID3D12Heap>> m_heaps;
    void Init(SkAgent *initAgent, std::string path, uint32_t mipLevels = 1)
    {
        agent = initAgent;
        base = agent->GetBase();
        // base=initBase;
        int _width, _height, nrChannels;
        unsigned char *data = stbi_load(path.c_str(), &_width, &_height, &nrChannels, 4);
        this->width = static_cast<uint32_t>(_width);
        this->height = static_cast<uint32_t>(_height);
        this->format = DXGI_FORMAT_R8G8B8A8_UNORM;
        this->mipLevels = mipLevels;
        if (!data)
        {
            throw std::runtime_error("Failed to load texture in " + path);
        }
        uint32_t size = this->width * this->height * Channels;
        pixelData.resize(size);
        memcpy(pixelData.data(), data, size * sizeof(unsigned char));
        stbi_image_free(data);
    }
    void Setup(int binding)
    {

        {
            textureDesc.MipLevels = this->mipLevels;
            textureDesc.Format = this->format;
            textureDesc.Width = this->width;
            textureDesc.Height = this->height;
            textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            textureDesc.DepthOrArraySize = 1;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

            if (mipLevels > 1)
            {
                textureDesc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
                SK_CHECK(base->device->CreateReservedResource(
                    &textureDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr,
                    IID_PPV_ARGS(&texture)));
            }
            else
            {
                SK_CHECK(base->device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                    D3D12_HEAP_FLAG_NONE,
                    &textureDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr,
                    IID_PPV_ARGS(&texture)));
            }

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);
            
            SK_CHECK(base->device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&textureUploadHeap)));

            // Describe and create a SRV for the texture.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = this->format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = this->mipLevels;
            base->device->CreateShaderResourceView(texture.Get(), &srvDesc, base->heap->GetSRV(binding));
        }
        if (this->mipLevels > 1)
        {
            uint32_t numTiles = 0;
            D3D12_TILE_SHAPE tileShape = {};
            uint32_t subresourceCount = this->mipLevels;
            std::vector<D3D12_SUBRESOURCE_TILING> tilings{subresourceCount};

            base->device->GetResourceTiling(texture.Get(), &numTiles, &packedMipInfo, &tileShape, &subresourceCount, 0, &tilings[0]);
            UINT heapCount = packedMipInfo.NumStandardMips + (packedMipInfo.NumPackedMips > 0 ? 1 : 0);
            fprintf(stderr, "numTiles:%d...subresourceCount:%d...\n", numTiles, subresourceCount);

            fprintf(stderr, "Mip: %d\t%d\t%d...\n", this->mipLevels, packedMipInfo.NumStandardMips, packedMipInfo.NumPackedMips);

            m_heaps.resize(heapCount);
            D3D12_TILE_RANGE_FLAGS flag = D3D12_TILE_RANGE_FLAG_NONE;
            uint32_t heapRangeStartOffset = 0;
            for (UINT n = 0; n < heapCount; n++)
            {
                D3D12_TILE_REGION_SIZE regionSize = {};
                if (n < packedMipInfo.NumStandardMips)
                {
                    regionSize.Width = tilings[n].WidthInTiles;
                    regionSize.Height = tilings[n].HeightInTiles;
                    regionSize.Depth = tilings[n].DepthInTiles;
                    regionSize.NumTiles = tilings[n].WidthInTiles * tilings[n].HeightInTiles * tilings[n].DepthInTiles;
                    regionSize.UseBox = true;
                }
                else
                {
                    regionSize.NumTiles = packedMipInfo.NumTilesForPackedMips;
                    regionSize.UseBox = false;
                }

                const UINT heapSize = regionSize.NumTiles * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

                CD3DX12_HEAP_DESC heapDesc(heapSize, D3D12_HEAP_TYPE_DEFAULT, 0, D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES);
                SK_CHECK(base->device->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_heaps[n])));

                CD3DX12_TILED_RESOURCE_COORDINATE coord{0, 0, 0, n};

                base->cmdQueue->UpdateTileMappings(texture.Get(), 1, &coord,
                                                   &regionSize, m_heaps[n].Get(),
                                                   1, &flag, &heapRangeStartOffset,
                                                   &regionSize.NumTiles,
                                                   D3D12_TILE_MAPPING_FLAG_NONE);
            }
            mip.resize(this->mipLevels - 1);
            uint32_t _width = this->width;
            uint32_t _height = this->height;
            this->Filter(pixelData, mip[0], _width, _height);
            for (size_t m = 1; m < mip.size(); m++)
            {
                this->Filter(mip[m-1], mip[m], _width, _height);
            }
        }

        {

            std::vector<D3D12_SUBRESOURCE_DATA> textureData{1};
            textureData[0].pData = pixelData.data();
            // textureData.pData = data;
            textureData[0].RowPitch = this->width * Channels;
            textureData[0].SlicePitch = textureData[0].RowPitch * this->height;
            fprintf(stderr, "Texture Size : %d,%d...\n", this->width, this->height);
            if(this->mipLevels>1)
            {
                textureData.resize(this->mipLevels);
                for (size_t n = 1; n < this->mipLevels; n++)
                {
                    textureData[n].pData=mip[n-1].data();
                    textureData[n].RowPitch=(this->width>>n) * Channels;
                    textureData[n].SlicePitch = textureData[n].RowPitch * (this->height>>n);
                }
            }
            {
                auto cmd = agent->BeginCmd();
                UpdateSubresources(cmd.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, static_cast<UINT>(textureData.size()), textureData.data());

                cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

                agent->FlushCmd(cmd);
            }
        }
    }
    void InitCheckerboard()
    {
        const UINT TextureWidth = 256;
        const UINT TextureHeight = 256;
        const UINT rowPitch = TextureWidth * Channels;
        const UINT cellPitch = rowPitch >> 3;      // The width of a cell in the checkboard texture.
        const UINT cellHeight = TextureWidth >> 3; // The height of a cell in the checkerboard texture.
        const UINT textureSize = rowPitch * TextureHeight;

        // std::vector<UINT8> data(textureSize);
        pixelData.resize(textureSize);
        for (UINT n = 0; n < textureSize; n += Channels)
        {
            UINT x = n % rowPitch;
            UINT y = n / rowPitch;
            UINT i = x / cellPitch;
            UINT j = y / cellHeight;

            if (i % 2 == j % 2)
            {
                pixelData[n] = 0x00;     // R
                pixelData[n + 1] = 0x00; // G
                pixelData[n + 2] = 0x00; // B
                pixelData[n + 3] = 0xff; // A
            }
            else
            {
                pixelData[n] = 0xff;     // R
                pixelData[n + 1] = 0xff; // G
                pixelData[n + 2] = 0xff; // B
                pixelData[n + 3] = 0xff; // A
            }
        }
        this->width = TextureWidth;
        this->height = TextureHeight;
        this->format = DXGI_FORMAT_R8G8B8A8_UNORM;
    }
    ~SkTex()
    {
    }
};
