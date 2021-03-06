#define _MApiVersion
#include "pch.h"
#include "msmayaUtils.h"
#include "MeshSyncClientMaya.h"

bool IsVisible(const MObject& node)
{
    Pad<MFnDagNode> dag(node);
    auto vis = dag.findPlug("visibility");
    bool visible = true;
    vis.getValue(visible);
    return visible;
}

std::string GetName(const MObject& node)
{
    MFnDependencyNode fn_node(node);
    return fn_node.name().asChar();
}
std::string GetPath(const MDagPath& path)
{
    std::string ret = path.fullPathName().asChar();
    std::replace(ret.begin(), ret.end(), '|', '/');
    return ret;
}
std::string GetPath(const MObject& node)
{
    return GetPath(GetDagPath(node));
}

std::string GetRootBonePath(const MObject& joint_)
{
    auto joint = joint_;
    for (;;) {
        MObject parent = GetParent(joint);
        if (parent.isNull() || !parent.hasFn(kMFnJoint))
            break;
        joint = parent;
    }
    return GetPath(joint);
}


MDagPath GetDagPath(const MObject& node)
{
    MDagPath ret;
    MDagPath::getAPathTo(node, ret);
    return ret;
}

MObject GetTransform(const MDagPath& path)
{
    return path.transform();
}
MObject GetTransform(const MObject& node)
{
    return GetTransform(GetDagPath(node));
}

MObject GetShape(const MDagPath& path_)
{
    auto path = path_;
    path.extendToShape();
    return path.node();
}

MObject GetShape(const MObject& node)
{
    auto path = GetDagPath(node);
    path.extendToShape();
    return path.node();
}

MDagPath GetParent(const MDagPath & node)
{
    MDagPath ret = node;
    ret.pop(1);
    return ret;
}

MObject GetParent(const MObject& node)
{
    Pad<MFnDagNode> dn(GetDagPath(node));
    return dn.parentCount() > 0 ? dn.parent(0) : MObject();
}

bool IsInstance(const MObject& node)
{
    Pad<MFnDagNode> dn(node);
    return dn.isInstanced(false);
}

MObject FindMesh(const MObject& node)
{
    auto path = GetDagPath(node);
    if (path.extendToShape() == MS::kSuccess) {
        auto shape = path.node();
        if (shape.hasFn(kMFnMesh)) {
            return shape;
        }
    }
    return MObject();
}

MObject FindSkinCluster(const MObject& node_)
{
    auto node = node_;
    if (!node.hasFn(kMFnMesh)) {
        node = FindMesh(GetTransform(node));
    }

    MItDependencyGraph it(node, kMFnSkinClusterFilter, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        return it.currentItem();
    }
    return MObject();
}

MObject FindBlendShape(const MObject& node_)
{
    auto node = node_;
    if (!node.hasFn(kMFnMesh)) {
        node = FindMesh(GetTransform(node));
    }

    MItDependencyGraph it(node, kMFnBlendShape, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        return it.currentItem();
    }
    return MObject();
}

MObject FindOrigMesh(const MObject& node)
{
    MObject ret;
    EachChild(node, [&](const MObject& child) {
        if (child.hasFn(kMFnMesh)) {
            MFnMesh fn(child);
            if (ret.isNull() || fn.isIntermediateObject()) {
                ret = child;
            }
        }
    });
    return ret;
}

float ToSeconds(MTime t)
{
    t.setUnit(MTime::kSeconds);
    return (float)t.value();
}

MTime ToMTime(float seconds)
{
    MTime ret;
    ret.setUnit(MTime::kSeconds);
    ret.setValue(seconds);
    return ret;
}


#ifdef mscDebug
static void DumpPlugInfoImpl(MPlug plug, std::string indent)
{
    uint32_t num_elements = 0;
    uint32_t num_children= 0;

    char array_info[64] = "";
    if (plug.isArray()) {
        num_elements = plug.numElements();
        sprintf(array_info, "[%d]", num_elements);
    }
    mscTrace("%splug %s%s\n", indent.c_str(), plug.name().asChar(), array_info);
    if (plug.isCompound()) {
        auto indent2 = indent + " ";
        num_children = plug.numChildren();
        if (plug.isArray()) {
            for (uint32_t i = 0; i < num_children; ++i) {
                DumpPlugInfo(plug.elementByPhysicalIndex(0).child(i), indent2);
            }
        }
        else {
            for (uint32_t i = 0; i < num_children; ++i) {
                DumpPlugInfo(plug.child(i), indent2);
            }
        }
    }
}

void DumpPlugInfoImpl(MPlug plug)
{
    DumpPlugInfoImpl(plug, "");
    mscTrace("\n");
}
#endif

