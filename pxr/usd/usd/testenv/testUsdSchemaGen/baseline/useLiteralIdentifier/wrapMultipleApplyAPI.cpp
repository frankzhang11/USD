//
// Copyright 2016 Pixar
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
#include "pxr/usd/usdContrived/multipleApplyAPI.h"
#include "pxr/usd/usd/schemaBase.h"

#include "pxr/usd/sdf/primSpec.h"

#include "pxr/usd/usd/pyConversions.h"
#include "pxr/base/tf/pyAnnotatedBoolResult.h"
#include "pxr/base/tf/pyContainerConversions.h"
#include "pxr/base/tf/pyResultConversions.h"
#include "pxr/base/tf/pyUtils.h"
#include "pxr/base/tf/wrapTypeHelpers.h"

#include <boost/python.hpp>

#include <string>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

namespace {

#define WRAP_CUSTOM                                                     \
    template <class Cls> static void _CustomWrapCode(Cls &_class)

// fwd decl.
WRAP_CUSTOM;

        
static UsdAttribute
_CreateTestAttrOneAttr(UsdContrivedMultipleApplyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateTestAttrOneAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int), writeSparsely);
}
        
static UsdAttribute
_CreateTestAttrTwoAttr(UsdContrivedMultipleApplyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateTestAttrTwoAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double), writeSparsely);
}

static bool _WrapIsMultipleApplyAPIPath(const SdfPath &path) {
    TfToken collectionName;
    return UsdContrivedMultipleApplyAPI::IsMultipleApplyAPIPath(
        path, &collectionName);
}

static std::string
_Repr(const UsdContrivedMultipleApplyAPI &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    std::string instanceName = self.GetName();
    return TfStringPrintf(
        "UsdContrived.MultipleApplyAPI(%s, '%s')",
        primRepr.c_str(), instanceName.c_str());
}

struct UsdContrivedMultipleApplyAPI_CanApplyResult : 
    public TfPyAnnotatedBoolResult<std::string>
{
    UsdContrivedMultipleApplyAPI_CanApplyResult(bool val, std::string const &msg) :
        TfPyAnnotatedBoolResult<std::string>(val, msg) {}
};

static UsdContrivedMultipleApplyAPI_CanApplyResult
_WrapCanApply(const UsdPrim& prim, const TfToken& name)
{
    std::string whyNot;
    bool result = UsdContrivedMultipleApplyAPI::CanApply(prim, name, &whyNot);
    return UsdContrivedMultipleApplyAPI_CanApplyResult(result, whyNot);
}

} // anonymous namespace

void wrapUsdContrivedMultipleApplyAPI()
{
    typedef UsdContrivedMultipleApplyAPI This;

    UsdContrivedMultipleApplyAPI_CanApplyResult::Wrap<UsdContrivedMultipleApplyAPI_CanApplyResult>(
        "_CanApplyResult", "whyNot");

    class_<This, bases<UsdAPISchemaBase> >
        cls("MultipleApplyAPI");

    cls
        .def(init<UsdPrim, TfToken>())
        .def(init<UsdSchemaBase const&, TfToken>())
        .def(TfTypePythonClass())

        .def("Get",
            (UsdContrivedMultipleApplyAPI(*)(const UsdStagePtr &stage, 
                                       const SdfPath &path))
               &This::Get,
            (arg("stage"), arg("path")))
        .def("Get",
            (UsdContrivedMultipleApplyAPI(*)(const UsdPrim &prim,
                                       const TfToken &name))
               &This::Get,
            (arg("prim"), arg("name")))
        .staticmethod("Get")

        .def("CanApply", &_WrapCanApply, (arg("prim"), arg("name")))
        .staticmethod("CanApply")

        .def("Apply", &This::Apply, (arg("prim"), arg("name")))
        .staticmethod("Apply")

        .def("GetSchemaAttributeNames",
             (const TfTokenVector &(*)(bool))&This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .def("GetSchemaAttributeNames",
             (TfTokenVector(*)(bool, const TfToken &))
                &This::GetSchemaAttributeNames,
             arg("includeInherited"),
             arg("instanceName"),
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetTestAttrOneAttr",
             &This::GetTestAttrOneAttr)
        .def("CreateTestAttrOneAttr",
             &_CreateTestAttrOneAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetTestAttrTwoAttr",
             &This::GetTestAttrTwoAttr)
        .def("CreateTestAttrTwoAttr",
             &_CreateTestAttrTwoAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

        .def("IsMultipleApplyAPIPath", _WrapIsMultipleApplyAPIPath)
            .staticmethod("IsMultipleApplyAPIPath")
        .def("__repr__", ::_Repr)
    ;

    _CustomWrapCode(cls);
}

// ===================================================================== //
// Feel free to add custom code below this line, it will be preserved by 
// the code generator.  The entry point for your custom code should look
// minimally like the following:
//
// WRAP_CUSTOM {
//     _class
//         .def("MyCustomMethod", ...)
//     ;
// }
//
// Of course any other ancillary or support code may be provided.
// 
// Just remember to wrap code in the appropriate delimiters:
// 'namespace {', '}'.
//
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

namespace {

WRAP_CUSTOM {
}

}
