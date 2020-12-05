// yabridge: a Wine VST bridge
// Copyright (C) 2020  Robbert van der Helm
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "plugin-factory.h"

YaPluginFactory::YaPluginFactory(){FUNKNOWN_CTOR}

YaPluginFactory::YaPluginFactory(Steinberg::IPluginFactory* /*factory*/){
    FUNKNOWN_CTOR

    // TODO: Copy everything from `factory`
}

YaPluginFactory::~YaPluginFactory() {
    FUNKNOWN_DTOR
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
IMPLEMENT_REFCOUNT(YaPluginFactory)
#pragma GCC diagnostic pop

tresult PLUGIN_API YaPluginFactory::queryInterface(Steinberg::FIDString _iid,
                                                   void** obj) {
    QUERY_INTERFACE(_iid, obj, Steinberg::FUnknown::iid,
                    Steinberg::IPluginFactory)
    QUERY_INTERFACE(_iid, obj, Steinberg::IPluginFactory::iid,
                    Steinberg::IPluginFactory)
    if (actual_iid == Steinberg::IPluginFactory2::iid ||
        actual_iid == Steinberg::IPluginFactory3::iid) {
        QUERY_INTERFACE(_iid, obj, Steinberg::IPluginFactory2::iid,
                        Steinberg::IPluginFactory2)
    }
    if (actual_iid == Steinberg::IPluginFactory3::iid) {
        QUERY_INTERFACE(_iid, obj, Steinberg::IPluginFactory3::iid,
                        Steinberg::IPluginFactory3)
    }

    *obj = nullptr;
    return Steinberg::kNoInterface;
}

tresult PLUGIN_API
YaPluginFactory::getFactoryInfo(Steinberg::PFactoryInfo* info) {
    // TODO: Implement
    return 0;
}

int32 PLUGIN_API YaPluginFactory::countClasses() {
    // TODO: Implement
    return 0;
}

tresult PLUGIN_API YaPluginFactory::getClassInfo(Steinberg::int32 index,
                                                 Steinberg::PClassInfo* info) {
    // TODO: Implement
    return 0;
}

tresult PLUGIN_API YaPluginFactory::createInstance(Steinberg::FIDString cid,
                                                   Steinberg::FIDString _iid,
                                                   void** obj) {
    // TODO: Implement
    return 0;
}

tresult PLUGIN_API
YaPluginFactory::getClassInfo2(int32 index, Steinberg::PClassInfo2* info) {
    // TODO: Implement
    return 0;
}

tresult PLUGIN_API
YaPluginFactory::getClassInfoUnicode(int32 index,
                                     Steinberg::PClassInfoW* info) {
    // TODO: Implement
    return 0;
}

tresult PLUGIN_API
YaPluginFactory::setHostContext(Steinberg::FUnknown* context) {
    // TODO: Implement
    return 0;
}
