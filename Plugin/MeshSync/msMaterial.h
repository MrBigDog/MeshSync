#pragma once

namespace ms {

class Texture : public std::enable_shared_from_this<Texture>
{
public:
    enum class TextureType
    {
        Default,
        NormalMap,
    };

    int id = 0;
    std::string filename;
    TextureType type = TextureType::Default;
    RawVector<uint8_t> data;

    static Texture* make(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
HasSerializer(Texture);
using TexturePtr = std::shared_ptr<Texture>;


class Material : public std::enable_shared_from_this<Material>
{
public:
    int id = 0;
    std::string name;

    float4 color = float4::one();
    float4 emission = float4::zero();
    float metalic = 0.0f;
    float smoothness = 0.5f;
    float opacity = 1.0f;

    static Material* make(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
HasSerializer(Material);
using MaterialPtr = std::shared_ptr<Material>;



} // namespace ms
