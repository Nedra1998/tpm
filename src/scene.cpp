#include "scene.hpp"

#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <tuple>

#include <pugixml.hpp>

#include "exit_code.hpp"
#include "log.hpp"
#include "prof.hpp"

cl::sycl::float3 tpm::parse_hex(const std::string &hex) {
  int r, g, b;
  sscanf(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
  return cl::sycl::float3(r / 255.0, g / 255.0, b / 255.0);
}

std::size_t tpm::parse_mat(const pugi::xml_node &node, TpmSpec &spec) {
  PFUNC(&node);
  for (const pugi::xml_node child : node) {
    std::string type = child.name();
    if (type == "emission") {
      spec.mats.emplace_back(MatType::EMISSION,
                             parse_hex(child.attribute("color").as_string()),
                             child.attribute("s").as_float());
      return spec.mats.size() - 1;
    } else if (type == "diffuse") {
    } else if (type == "glass") {
    } else if (type == "glossy") {
    }
  }
  return std::numeric_limits<std::size_t>::max();
}

std::size_t tpm::parse_sphere(const pugi::xml_node &node, TpmSpec &spec) {
  PFUNC(&node);
  spec.sdfs.emplace_back(SdfType::SPHERE, node.attribute("r").as_float());
  spec.sdfs.back().mat = parse_mat(node, spec);
  return spec.sdfs.size() - 1;
}

std::size_t tpm::parse_translate(const pugi::xml_node &node, TpmSpec &spec) {
  PFUNC(&node);
  spec.sdfs.emplace_back(SdfType::TRANSLATE, node.attribute("x").as_float(),
                         node.attribute("y").as_float(),
                         node.attribute("z").as_float());
  std::size_t id = spec.sdfs.size() - 1;
  spec.sdfs[id].a = parse_sdf(*node.begin(), spec);
  return id;
}

std::size_t tpm::parse_union(const pugi::xml_node &node, TpmSpec &spec) {
  PFUNC(&node);
  spec.sdfs.emplace_back(SdfType::UNION);
  std::size_t id = spec.sdfs.size() - 1;
  spec.sdfs[id].a = parse_sdf(*node.begin(), spec);
  spec.sdfs[id].b = parse_sdf(*(++node.begin()), spec);
  return id;
}

std::size_t tpm::parse_sdf(const pugi::xml_node &node, TpmSpec &spec) {
  PFUNC(&node);
  std::string type = node.name();
  if (type == "sphere") {
    return parse_sphere(node, spec);
  } else if (type == "translate") {
    return parse_translate(node, spec);
  } else if (type == "union") {
    return parse_union(node, spec);
  } else {
    LWARN("Unknown node type \"{}\", ignoring", type);
    return std::numeric_limits<std::size_t>::max();
  }
}

std::pair<tpm::ExitCode, tpm::TpmSpec>
tpm::parse_spec(const std::string &path) {
  PFUNC(path);

  TpmSpec tpm_spec;

  if (!std::filesystem::exists(path)) {
    LERR("Scene file \"{}\" does not exist!", path);
    return std::make_pair(SCENE_NOT_FOUND, std::move(tpm_spec));
  }

  pugi::xml_document doc;
  if (!doc.load_file(path.c_str())) {
    LERR("Failed to parse scene file \"{}\"", path);
    return std::make_pair(SCENE_PARSE_ERROR, std::move(tpm_spec));
  }

  const pugi::xml_node root = doc.child("tpm");
  if (!root) {
    LERR(
        "`tpm` must be the root node in the scene description file file \"{}\"",
        path);
    return std::make_pair(SCENE_PARSE_ERROR, std::move(tpm_spec));
  }

  pugi::xml_node image = root.child("image");
  if (image) {
    tpm_spec.image = ImageSpec{
        image.attribute("path").as_string("output.png"),
        image.attribute("width").as_uint(1920),
        image.attribute("height").as_uint(1080),
        image.attribute("tileSize").as_uint(32),
    };
  }

  pugi::xml_node renderer = root.child("renderer");
  if (renderer) {
    tpm_spec.renderer = RendererSpec{
        image.attribute("spp").as_ullong(64),
    };
  }

  pugi::xml_node scene = root.child("scene");
  if (scene) {
    parse_sdf(*scene.begin(), tpm_spec);
  } else {
    LERR("SDF not present in scene description file");
    return std::make_pair(SCENE_MISSING, std::move(tpm_spec));
  }

  return std::make_pair(OK, std::move(tpm_spec));
}
