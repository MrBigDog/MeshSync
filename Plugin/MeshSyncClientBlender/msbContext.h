#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "msbBinder.h"

struct msbSettings;
class msbContext;
namespace bl = blender;


struct msbSettings
{
    ms::ClientSettings client_settings;
    ms::SceneSettings scene_settings;
    bool sync_meshes = true;
    bool sync_normals = true;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool sync_bones = true;
    bool sync_blendshapes = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool calc_per_index_normals = true;

    float animation_timescale = 1.0f;
    int animation_frame_interval = 10;
};


class msbContext : public std::enable_shared_from_this<msbContext>
{
public:
    enum class SendScope
    {
        None,
        All,
        Updated,
        Selected,
    };

    msbContext();
    ~msbContext();

    msbSettings&        getSettings();
    const msbSettings&  getSettings() const;

    bool isSending() const;
    bool prepare();
    void sendScene(SendScope scope);
    void sendAnimations(SendScope scope);
    void flushPendingList();

private:
    struct ObjectRecord : public mu::noncopyable
    {
        std::string name;
        std::string path;
        bool alive = false;
        bool exported = false;

        void clear()
        {
            alive = false;
            exported = false;
        }
    };

    struct AnimationRecord : public mu::noncopyable
    {
        using extractor_t = void (msbContext::*)(ms::Animation& dst, void *obj);

        void *obj = nullptr;
        ms::Animation *dst = nullptr;
        extractor_t extractor = nullptr;

        void operator()(msbContext *_this)
        {
            (_this->*extractor)(*dst, obj);
        }
    };

    ms::TransformPtr    addTransform(std::string path);
    ms::CameraPtr       addCamera(std::string path);
    ms::LightPtr        addLight(std::string path);
    ms::MeshPtr         addMesh(std::string path);
    void                addDeleted(const std::string& path);
    ms::MaterialPtr     addMaterial(Material *material);

    int getMaterialIndex(const Material *mat);
    void extractTransformData(ms::Transform& dst, Object *obj);
    void extractCameraData(ms::Camera& dst, Object *obj);
    void extractLightData(ms::Light& dst, Object *obj);
    void extractMeshData(ms::Mesh& dst, Object *obj);

    void exportMaterials();
    ms::TransformPtr exportArmature(Object *obj);
    ms::TransformPtr exportObject(Object *obj, bool force);
    ms::TransformPtr exportReference(Object *obj, const std::string& base_path);
    ms::TransformPtr exportDupliGroup(Object *obj, const std::string & base_path);
    ObjectRecord& touchRecord(Object *obj);
    void eraseStaleObjects();

    ms::TransformPtr findBone(const Object *armature, const Bone *bone);

    void doExtractMeshData(ms::Mesh& mesh, Object *obj);
    void doExtractNonEditMeshData(ms::Mesh& mesh, Object *obj);
    void doExtractEditMeshData(ms::Mesh& mesh, Object *obj);

    void exportAnimation(Object *obj, bool force, const std::string base_path="");
    void extractTransformAnimationData(ms::Animation& dst, void *obj);
    void extractPoseAnimationData(ms::Animation& dst, void *obj);
    void extractCameraAnimationData(ms::Animation& dst, void *obj);
    void extractLightAnimationData(ms::Animation& dst, void *obj);
    void extractMeshAnimationData(ms::Animation& dst, void *obj);

    void kickAsyncSend();

private:
    msbSettings m_settings;
    std::set<Object*> m_added;
    std::set<Object*> m_pending, m_pending_tmp;
    std::map<const Bone*, ms::TransformPtr> m_bones;
    std::vector<ms::TransformPtr> m_objects;
    std::vector<ms::MeshPtr> m_meshes;
    std::vector<ms::AnimationClipPtr> m_animations;
    std::vector<ms::MaterialPtr> m_materials;
    std::vector<std::string> m_deleted;
    std::map<void*, ObjectRecord> m_obj_records;

    std::future<void> m_send_future;

    using task_t = std::function<void()>;
    std::vector<task_t> m_extract_tasks;

    // animation export
    using AnimationRecords = std::map<std::string, AnimationRecord>;
    AnimationRecords m_anim_records;
    float m_current_time = 0.0f;
    bool m_ignore_update = false;
};
using msbContextPtr = std::shared_ptr<msbContext>;
