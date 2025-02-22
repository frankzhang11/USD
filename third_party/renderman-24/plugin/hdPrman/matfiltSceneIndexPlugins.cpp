//
// Copyright 2022 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "hdPrman/matfiltSceneIndexPlugins.h"
#include "hdPrman/material.h"
#include "hdPrman/matfiltFilterChain.h"
#include "hdPrman/matfiltConvertPreviewMaterial.h"

#ifdef PXR_MATERIALX_SUPPORT_ENABLED
#include "hdPrman/matfiltMaterialX.h"
#endif

#include "hdPrman/virtualStructResolvingSceneIndex.h"

#include "pxr/base/tf/stringUtils.h"

#include "pxr/imaging/hd/dataSourceTypeDefs.h"
#include "pxr/imaging/hd/materialFilteringSceneIndexBase.h"
#include "pxr/imaging/hd/materialNetworkInterface.h"
#include "pxr/imaging/hd/retainedDataSource.h"
#include "pxr/imaging/hd/sceneIndexPluginRegistry.h"

#include <string>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (applyConditionals)
    ((previewMatPluginName, "HdPrman_PreviewMaterialFilteringSceneIndexPlugin"))
    ((materialXPluginName,  "HdPrman_MaterialXFilteringSceneIndexPlugin"))
    ((vstructPluginName,    "HdPrman_VirtualStructResolvingSceneIndexPlugin"))
);

////////////////////////////////////////////////////////////////////////////////
// Plugin registrations
////////////////////////////////////////////////////////////////////////////////

static const char * const _rendererDisplayName = "Prman";
// XXX: Hardcoded for now to match the legacy matfilt logic.
static const bool _resolveVstructsWithConditionals = true;

TF_REGISTRY_FUNCTION(TfType)
{
    HdSceneIndexPluginRegistry::Define<
        HdPrman_PreviewMaterialFilteringSceneIndexPlugin,
        HdSceneIndexPlugin>();
    
    HdSceneIndexPluginRegistry::Define<
        HdPrman_MaterialXFilteringSceneIndexPlugin,
        HdSceneIndexPlugin>();
    
    HdSceneIndexPluginRegistry::Define<
        HdPrman_VirtualStructResolvingSceneIndexPlugin,
        HdSceneIndexPlugin>();
}

TF_REGISTRY_FUNCTION(HdSceneIndexPlugin)
{
    // Register the plugins conditionally.
    if (HdPrmanMaterial::GetUseSceneIndexForMatfilt()) {
        HdSceneIndexPluginRegistry::GetInstance().RegisterSceneIndexForRenderer(
            _rendererDisplayName,
            _tokens->previewMatPluginName,
            nullptr, // no argument data necessary
            MatfiltOrder::NodeTranslation,
            HdSceneIndexPluginRegistry::InsertionOrderAtStart);
        
        HdSceneIndexPluginRegistry::GetInstance().RegisterSceneIndexForRenderer(
            _rendererDisplayName,
            _tokens->materialXPluginName,
            nullptr, // no argument data necessary
            MatfiltOrder::NodeTranslation,
            HdSceneIndexPluginRegistry::InsertionOrderAtStart);
        
        HdContainerDataSourceHandle const inputArgs =
            HdRetainedContainerDataSource::New(
                _tokens->applyConditionals,
                HdRetainedTypedSampledDataSource<bool>::New(
                    _resolveVstructsWithConditionals));

        HdSceneIndexPluginRegistry::GetInstance().RegisterSceneIndexForRenderer(
            _rendererDisplayName,
            _tokens->vstructPluginName,
            inputArgs,                        
            MatfiltOrder::ConnectionResolve,
            HdSceneIndexPluginRegistry::InsertionOrderAtStart);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Scene Index Implementations
////////////////////////////////////////////////////////////////////////////////

namespace
{

void
_TransformPreviewMaterialNetwork(
    HdMaterialNetworkInterface *networkInterface)
{
    std::vector<std::string> errors;
    MatfiltConvertPreviewMaterial(networkInterface, &errors);
    if (!errors.empty()) {
        TF_RUNTIME_ERROR(
            "Error filtering preview material network for prim %s: %s\n",
                networkInterface->GetMaterialPrimPath().GetText(),
                TfStringJoin(errors).c_str());
    }
}

TF_DECLARE_REF_PTRS(_PreviewMaterialFilteringSceneIndex);

class _PreviewMaterialFilteringSceneIndex :
    public HdMaterialFilteringSceneIndexBase
{
public:

    static _PreviewMaterialFilteringSceneIndexRefPtr New(
        const HdSceneIndexBaseRefPtr &inputScene) 
    {
        return TfCreateRefPtr(
            new _PreviewMaterialFilteringSceneIndex(inputScene));
    }

protected:
    _PreviewMaterialFilteringSceneIndex(
        const HdSceneIndexBaseRefPtr &inputSceneIndex)
    : HdMaterialFilteringSceneIndexBase(inputSceneIndex)
    {
    }

    FilteringFnc _GetFilteringFunction() const override
    {
        return _TransformPreviewMaterialNetwork;
    }
};

/// ----------------------------------------------------------------------------

#ifdef PXR_MATERIALX_SUPPORT_ENABLED

void
_TransformMaterialXNetwork(
    HdMaterialNetworkInterface *networkInterface)
{
    std::vector<std::string> errors;
    MatfiltMaterialX(networkInterface, &errors);
    if (!errors.empty()) {
        TF_RUNTIME_ERROR(
            "Error filtering preview material network for prim %s: %s\n",
                networkInterface->GetMaterialPrimPath().GetText(),
                TfStringJoin(errors).c_str());
    }
}

TF_DECLARE_REF_PTRS(_MaterialXFilteringSceneIndex);

class _MaterialXFilteringSceneIndex :
    public HdMaterialFilteringSceneIndexBase
{
public:

    static _MaterialXFilteringSceneIndexRefPtr New(
        const HdSceneIndexBaseRefPtr &inputScene) 
    {
        return TfCreateRefPtr(
            new _MaterialXFilteringSceneIndex(inputScene));
    }

protected:
    _MaterialXFilteringSceneIndex(
        const HdSceneIndexBaseRefPtr &inputSceneIndex)
    : HdMaterialFilteringSceneIndexBase(inputSceneIndex)
    {
    }

    FilteringFnc _GetFilteringFunction() const override
    {
        return _TransformMaterialXNetwork;
    }
};

#endif

/// ----------------------------------------------------------------------------

// Note: HdPrman_VirtualStructResolvingSceneIndex is defined in its own
// translation unit for unit testing purposes.
// 

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
// Scene Index Plugin Implementations
////////////////////////////////////////////////////////////////////////////////

HdPrman_PreviewMaterialFilteringSceneIndexPlugin::
HdPrman_PreviewMaterialFilteringSceneIndexPlugin() = default;

HdSceneIndexBaseRefPtr
HdPrman_PreviewMaterialFilteringSceneIndexPlugin::_AppendSceneIndex(
        const HdSceneIndexBaseRefPtr &inputScene,
        const HdContainerDataSourceHandle &inputArgs)
{
    TF_UNUSED(inputArgs);
    return _PreviewMaterialFilteringSceneIndex::New(inputScene);
}

/// ----------------------------------------------------------------------------

HdPrman_MaterialXFilteringSceneIndexPlugin::
HdPrman_MaterialXFilteringSceneIndexPlugin() = default;

HdSceneIndexBaseRefPtr
HdPrman_MaterialXFilteringSceneIndexPlugin::_AppendSceneIndex(
        const HdSceneIndexBaseRefPtr &inputScene,
        const HdContainerDataSourceHandle &inputArgs)
{
    TF_UNUSED(inputArgs);
#if PXR_MATERIALX_SUPPORT_ENABLED
    return _MaterialXFilteringSceneIndex::New(inputScene);
#else
    return inputScene;
#endif
}

/// ----------------------------------------------------------------------------

HdPrman_VirtualStructResolvingSceneIndexPlugin::
HdPrman_VirtualStructResolvingSceneIndexPlugin() = default;

HdSceneIndexBaseRefPtr
HdPrman_VirtualStructResolvingSceneIndexPlugin::_AppendSceneIndex(
        const HdSceneIndexBaseRefPtr &inputScene,
        const HdContainerDataSourceHandle &inputArgs)
{
    bool applyConditionals = false;
    if (!inputArgs->Has(_tokens->applyConditionals)) {
        TF_CODING_ERROR("Missing argument to plugin %s",
                        _tokens->vstructPluginName.GetText());
    } else {
        HdBoolDataSourceHandle val =
            HdBoolDataSource::Cast(inputArgs->Get(_tokens->applyConditionals));
        applyConditionals = val->GetTypedValue(0.0f);
    }
    return HdPrman_VirtualStructResolvingSceneIndex::New(
                inputScene, applyConditionals);
}

PXR_NAMESPACE_CLOSE_SCOPE
