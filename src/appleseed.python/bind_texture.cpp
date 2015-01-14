
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2012-2013 Esteban Tovagliari, Jupiter Jazz Limited
// Copyright (c) 2014-2015 Esteban Tovagliari, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Boost headers.
#include "foundation/platform/pythonheaderguards.h"
BEGIN_PYTHON_INCLUDES
#include "boost/python/detail/wrap_python.hpp"  // has to be first, to avoid redefinition warnings
#include "boost/python.hpp"
END_PYTHON_INCLUDES

// appleseed.python headers.
#include "bind_auto_release_ptr.h"
#include "bind_typed_entity_containers.h"
#include "dict2dict.h"
#include "metadata.h"
#include "unaligned_transformd44.h"

// appleseed.renderer headers.
#include "renderer/api/texture.h"
#include "renderer/modeling/scene/textureinstance.h"

// appleseed.foundation headers.
#include "foundation/utility/searchpaths.h"

namespace bpy = boost::python;
using namespace foundation;
using namespace renderer;

namespace
{
    auto_release_ptr<Texture> create_texture(
        const std::string&              texture_type,
        const std::string&              name,
        const bpy::dict&                params,
        const bpy::list&                search_paths)
    {
        TextureFactoryRegistrar factories;
        const ITextureFactory* factory = factories.lookup(texture_type.c_str());

        if (factory)
        {
            SearchPaths paths;

            for (bpy::ssize_t i = 0, e = bpy::len(search_paths); i < e; ++i)
            {
                bpy::extract<const char*> extractor(search_paths[i]);
                if (extractor.check())
                    paths.push_back(extractor());
                else
                {
                    PyErr_SetString(PyExc_TypeError, "Incompatible type. Only strings accepted.");
                    bpy::throw_error_already_set();
                }
            }

            return factory->create(name.c_str(), bpy_dict_to_param_array(params), paths);
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, "EDF type not found");
            bpy::throw_error_already_set();
        }

        return auto_release_ptr<Texture>();
    }

    auto_release_ptr<TextureInstance> create_texture_instance(
        const std::string&              name,
        const bpy::dict&                params,
        const std::string&              texture_name,
        const UnalignedTransformd44&    transform)
    {
        return
            TextureInstanceFactory::create(
                name.c_str(),
                bpy_dict_to_param_array(params),
                texture_name.c_str(),
                transform.as_foundation_transform());
    }

    UnalignedTransformd44 texture_inst_get_transform(const TextureInstance* tx)
    {
        return UnalignedTransformd44(tx->get_transform());
    }
}

void bind_texture()
{
    bpy::class_<Texture, auto_release_ptr<Texture>, bpy::bases<Entity>, boost::noncopyable>("Texture", bpy::no_init)
        .def("get_input_metadata", &detail::get_entity_input_metadata<TextureFactoryRegistrar>).staticmethod("get_input_metadata")
        .def("__init__", bpy::make_constructor(create_texture))
        .def("get_model", &Texture::get_model)
        ;

    bind_typed_entity_vector<Texture>("TextureContainer");

    bpy::class_<TextureInstance, auto_release_ptr<TextureInstance>, bpy::bases<Entity>, boost::noncopyable>("TextureInstance", bpy::no_init)
        .def("__init__", bpy::make_constructor(create_texture_instance))
        .def("get_transform", &texture_inst_get_transform)
        ;

    bind_typed_entity_vector<TextureInstance>("TextureInstanceContainer");
}
