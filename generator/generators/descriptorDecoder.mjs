import fs from "fs";
import nunjucks from "nunjucks";

import pkg from "../../package.json";

import {
  warn,
  isQuotedString,
  getEnumNameFromDawnEnumName
} from "../utils.mjs";

import {
  getASTNodeByName,
  getExplortDeclarationName
} from "../types.mjs";

let ast = null;

const DEFAULT_OPTS_DECODE_STRUCT_MEMBER = {
  padding: `    `,
  input: { name: "obj" },
  output: { name: "descriptor" }
};

const DEFAULT_OPTS_RESET_STRUCT = {
  padding: `  `,
  output: { name: "descriptor" }
};

const H_TEMPLATE = fs.readFileSync(`${pkg.config.TEMPLATE_DIR}/DescriptorDecoder-h.njk`, "utf-8");
const CPP_TEMPLATE = fs.readFileSync(`${pkg.config.TEMPLATE_DIR}/DescriptorDecoder-cpp.njk`, "utf-8");

nunjucks.configure({ autoescape: true });

export function getDecodeStructureMember(structure, member, opts = DEFAULT_OPTS_DECODE_STRUCT_MEMBER, insideDecoder = false) {
  let {type} = member;
  let {jsType, rawType} = type;
  let {input, output, padding} = opts;
  let out = ``;
  if (member.isInternalProperty) return out;
  if (type.isOptional) {
    out += `\n${padding}if (${input.name}.Has("${member.name}")) {`;
    padding = opts.padding += `  `;
  } else if (type.isArray) {
    out += `\n${padding}{`;
    padding = opts.padding += `  `;
  }
  // validate array
  if (
    type.isArray &&
    !jsType.isString &&
    !jsType.isTypedArray &&
    !jsType.isArrayBuffer
  ) {
    // loop through array items
    out += `
${padding}{
${padding}  Napi::Array array = ${input.name}.Get("${member.name}").As<Napi::Array>();
${padding}  uint32_t length = array.Length();
${padding}  for (unsigned int ii = 0; ii < length; ++ii) {`;
    // validate object
    if (type.isStructure) {
      out += `\n${padding}    if (!(array.Get(ii).IsObject())) {
${padding}      Napi::String type = Napi::String::New(value.Env(), "Type");
${padding}      Napi::String message = Napi::String::New(value.Env(), "Expected 'Object' for '${structure.externalName}'.'${member.name}'");
${padding}      device->throwCallbackError(type, message);
${padding}      return ${insideDecoder ? "descriptor" : ""};
${padding}    }`;
    }
    // validate class
    else if (type.isObject) {
      let unwrapType = getExplortDeclarationName(type.nativeType);
      out += `\n${padding}    if (!(array.Get(ii).IsObject()) || !(array.Get(ii).As<Napi::Object>().InstanceOf(${unwrapType}::constructor.Value()))) {
${padding}      Napi::String type = Napi::String::New(value.Env(), "Type");
${padding}      Napi::String message = Napi::String::New(value.Env(), "Expected '${unwrapType}' for '${structure.externalName}'.'${member.name}'");
${padding}      device->throwCallbackError(type, message);
${padding}      return ${insideDecoder ? "descriptor" : ""};
${padding}    }`;
    }
    else {
      warn(`Cannot validate type of array member '${structure.externalName}'.'${member.name}'`);
    }
    out += `
${padding}  };
${padding}}`;
  }
  // validate primitives
  else if (
    jsType.isNumber ||
    jsType.isString ||
    jsType.isObject ||
    jsType.isBoolean ||
    jsType.isArrayBuffer ||
    jsType.isTypedArray
  ) {
    let strType = jsType.type;
    // napi check for typed arrays is .IsTypedArray()
    if (jsType.isTypedArray) {
      strType = "TypedArray";
    }
    // class-based type check
    if (jsType.isObject && type.isObject) {
      let unwrapType = getExplortDeclarationName(type.nativeType);
      out += `\n${padding}if (!(${input.name}.Get("${member.name}").IsObject()) || !(${input.name}.Get("${member.name}").As<Napi::Object>().InstanceOf(${unwrapType}::constructor.Value()))) {
${padding}  Napi::String type = Napi::String::New(value.Env(), "Type");
${padding}  Napi::String message = Napi::String::New(value.Env(), "Expected '${unwrapType}' for '${structure.externalName}'.'${member.name}'");
${padding}  device->throwCallbackError(type, message);
${padding}  return ${insideDecoder ? "descriptor" : ""};
${padding}}`;
    }
    // primitive type check
    else {
      out += `\n${padding}if (!(${input.name}.Get("${member.name}").Is${strType}())) {
${padding}  Napi::String type = Napi::String::New(value.Env(), "Type");
${padding}  Napi::String message = Napi::String::New(value.Env(), "Expected '${strType}' for '${structure.externalName}'.'${member.name}'");
${padding}  device->throwCallbackError(type, message);
${padding}  return ${insideDecoder ? "descriptor" : ""};
${padding}}`;
    }
  } else {
    warn(`Cannot validate type of member '${structure.externalName}'.'${member.name}'`);
  }
  // decode (unwrap) object typed members
  if (type.isObject && !type.isArray) {
    let unwrapType = getExplortDeclarationName(type.nativeType);
    out += `\n${padding}${output.name}.${member.name} = Napi::ObjectWrap<${unwrapType}>::Unwrap(${input.name}.Get("${member.name}").As<Napi::Object>())->instance;`;
  // decode descriptor object array
  } else if (type.isObject && type.isArray) {
    let unwrapType = getExplortDeclarationName(type.nativeType);
    out += `
${padding}Napi::Array array = ${input.name}.Get("${member.name}").As<Napi::Array>();
${padding}uint32_t length = array.Length();
${padding}${type.nativeType}* data = (${type.nativeType}*) malloc(length * sizeof(${type.nativeType}));
${padding}for (unsigned int ii = 0; ii < length; ++ii) {
${padding}  Napi::Object item = array.Get(ii).As<Napi::Object>();
${padding}  ${type.nativeType} value = Napi::ObjectWrap<${unwrapType}>::Unwrap(item)->instance;
${padding}  data[ii] = value;
${padding}};`;
    out += `
${padding}${output.name}.${type.length} = length;
${padding}${output.name}.${member.name} = data;`;
  // decode descriptor structure
  } else if (type.isStructure && !type.isArray) {
    let memberTypeStructure = ast.structures.filter(s => s.name === type.nativeType)[0] || null;
    if (!memberTypeStructure) {
      warn(`Cannot resolve relative structure of member '${structure.externalName}'.'${member.name}'`);
    }
    // recursively call this method, but modify 'opts' argument
    let nextOpts = {
      padding: opts.padding + `  `,
      input: { name: `$${member.name}` },
      output: { name: output.name + "." + member.name }
    };
    // struct member is a reference
    if (type.isReference) {
      nextOpts.output = { name: member.name };
      out += `\n${padding}  ${type.nativeType} ${member.name};`;
      // reset descriptor
      let resetOpts = {
        padding: padding + "  ",
        output: { name: member.name }
      };
      out += getDescriptorInstanceReset(memberTypeStructure, resetOpts);
    } else {
      // reset descriptor
      let resetOpts = {
        padding: padding + "  ",
        output: { name: output.name + "." + member.name }
      };
      out += getDescriptorInstanceReset(memberTypeStructure, resetOpts);
    }
    out += `\n${padding}  Napi::Object $${member.name} = ${input.name}.Get("${member.name}").As<Napi::Object>();`;
    memberTypeStructure.children.map(member => {
      out += getDecodeStructureMember(memberTypeStructure, member, nextOpts, insideDecoder);
    });
    // link the struct member reference to top structure
    if (type.isReference) {
      out += `\n${padding}{`;
      out += `\n${padding}  ${output.name}.${member.name} = (${type.nativeType}*) malloc(sizeof(${type.nativeType}));`;
      out += `\n${padding}  memcpy(const_cast<${type.nativeType}*>(${output.name}.${member.name}), &${member.name}, sizeof(${type.nativeType}));`;
      out += `\n${padding}}`;
    }
  // decode descriptor structure array
  } else if (type.isStructure && type.isArray) {
    let memberTypeStructure = ast.structures.filter(s => s.name === type.nativeType)[0] || null;
    if (!memberTypeStructure) {
      warn(`Cannot resolve relative structure of member '${structure.externalName}'.'${member.name}'`);
    }
    out += `
${padding}Napi::Array array = ${input.name}.Get("${member.name}").As<Napi::Array>();
${padding}uint32_t length = array.Length();`;

    if (type.isArrayOfPointers) {
      out += `
${padding}${type.nativeType}** data = (${type.nativeType}**) malloc(sizeof(${type.nativeType}*));`;
    } else {
      out += `
${padding}${type.nativeType}* data = (${type.nativeType}*) malloc(length * sizeof(${type.nativeType}));`;
    }

    out += `
${padding}for (unsigned int ii = 0; ii < length; ++ii) {
${padding}  Napi::Object item = array.Get(ii).As<Napi::Object>();
${padding}  ${type.nativeType} $${member.name} = Decode${memberTypeStructure.externalName}(device, item.As<Napi::Value>());`;

    // array of pointers to structs
    if (type.isArrayOfPointers) {
    out += `
${padding}  data[ii] = (${type.nativeType}*) malloc(sizeof(${type.nativeType}));
${padding}  memcpy(
${padding}    reinterpret_cast<void*>(data[ii]),
${padding}    reinterpret_cast<void*>(&$${member.name}),
${padding}    sizeof(${type.nativeType})
${padding}  );
${padding}};
${padding}${output.name}.${type.length} = length;
${padding}${output.name}.${member.name} = data;`;
    }
    // array of structs
    else {
    out += `
${padding}  memcpy(
${padding}    reinterpret_cast<void*>(&data[ii]),
${padding}    reinterpret_cast<void*>(&$${member.name}),
${padding}    sizeof(${type.nativeType})
${padding}  );
${padding}};
${padding}${output.name}.${type.length} = length;
${padding}${output.name}.${member.name} = data;`;
    }

  // decode numeric typed members
  } else if (type.isNumber) {
    // validate input and type
    {
      out += `\n${padding}if (!(${input.name}.Get("${member.name}").IsNumber())) {
${padding}  Napi::String type = Napi::String::New(value.Env(), "Type");
${padding}  Napi::String message = Napi::String::New(value.Env(), "Expected 'Number' for '${structure.externalName}'.'${member.name}'");
${padding}  device->throwCallbackError(type, message);
${padding}  return ${insideDecoder ? "descriptor" : ""};
${padding}}`;
    }
    switch (rawType) {
      case "int32_t": {
        out += `\n${padding}${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().Int32Value();`;
      } break;
      case "uint32_t": {
        out += `\n${padding}${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().Uint32Value();`;
      } break;
      case "float": {
        out += `\n${padding}${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().FloatValue();`;
      } break;
      case "uint64_t": {
        out += `\n${padding}{`;
        out += `\n${padding}  ${output.name}.${member.name} = static_cast<uint64_t>(${input.name}.Get("${member.name}").As<Napi::Number>().Uint32Value());`;
        out += `\n${padding}}`;
      } break;
      case "const uint32_t*": {
        /*out += `\n      size_t size;`;
        out += `\n      ${output.name}.${member.name} = getTypedArrayData<uint32_t>(${input.name}.Get("${member.name}"), &size);`;
        out += `\n      ${output.name}.${type.length} = static_cast<uint32_t>(size);`;*/
      } break;
      default: {
        warn(`Unexpected member type '${rawType}' in '${structure.externalName}'.'${member.name}'`);
      } break;
    };
  // decode boolean member
  } else if (type.isBoolean && !type.isArray) {
    out += `\n${padding}${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Boolean>().Value();`;
  // decode boolean member array
  } else if (type.isBoolean && type.isArray) {
    warn(`Unimplemented Boolean Array member: '${structure.externalName}'.'${member.name}'`);
  // decode enum member
  } else if (type.isEnum && !type.isArray) {
    let decodeMap = getExplortDeclarationName(type.nativeType);
    out += `\n${padding}${output.name}.${member.name} = `;
    out += `static_cast<${type.nativeType}>(`;
    out += `${decodeMap}(${input.name}.Get("${member.name}").As<Napi::String>().Utf8Value())`;
    out += `);`;
  // decode enum member array
  } else if (type.isEnum && type.isArray) {
    let decodeMap = getExplortDeclarationName(type.nativeType);
    out += `
${padding}Napi::Array array = ${input.name}.Get("${member.name}").As<Napi::Array>();
${padding}uint32_t length = array.Length();
${padding}${type.nativeType}* data = (${type.nativeType}*) malloc(length * sizeof(${type.nativeType}));
${padding}for (unsigned int ii = 0; ii < length; ++ii) {
${padding}  Napi::Object item = array.Get(ii).As<Napi::Object>();
${padding}  ${type.nativeType} value = static_cast<${type.nativeType}>(
${padding}    ${decodeMap}(item.As<Napi::String>().Utf8Value())
${padding}  );
${padding}  data[ii] = value;
${padding}};`;
    out += `
${padding}${output.name}.${member.name} = data;`;
  // decode bitmask member
  } else if (type.isBitmask && !type.isArray) {
    out += `\n${padding}${output.name}.${member.name} = static_cast<${type.nativeType}>(${input.name}.Get("${member.name}").As<Napi::Number>().Uint32Value());`;
  // decode bitmask member array
  } else if (type.isBitmask && type.isArray) {
    warn(`Unimplemented Bitmask Array member: '${structure.externalName}'.'${member.name}'`);
  // decode string member
  } else if (type.isString) {
    if (type.isDynamicLength) {
      out += `\n${padding}${output.name}.${member.name} = getNAPIStringCopy(${input.name}.Get("${member.name}"));`;
    } else {
      warn(`Cannot handle fixed String length in '${structure.externalName}'.'${member.name}'`);
    }
  // typed array
  } else if (type.isArray && jsType.isTypedArray) {
out += `
${padding}Napi::TypedArray array = ${input.name}.Get("${member.name}").As<Napi::TypedArray>();
${padding}Napi::ArrayBuffer buffer = array.ArrayBuffer();
${padding}${output.name}.${member.name} = reinterpret_cast<${rawType}>(buffer.Data());`;
  } else {
    warn(`Unexpected member type '${rawType}' in '${structure.externalName}'.'${member.name}'`);
  }
  if (type.isOptional || type.isArray) {
    padding = opts.padding = opts.padding.substr(0, opts.padding.length - 2);
    out += `\n${padding}}`;
  }
  return out;
};

function getDestroyStructureMember(structure, member) {
  let {type} = member;
  let {jsType, rawType, nativeType} = type;
  let out = ``;
  if (member.isInternalProperty) return out;
  if (type.isObject && !type.isArray) {
    // no need to free
  }
  else if (type.isObject && type.isArray) {
    out += `
    if (descriptor.${member.name}) {
      free((void*) const_cast<${nativeType}*>(descriptor.${member.name}));
    }`;
  }
  else if (type.isStructure && !type.isArray) {
    let exportType = getExplortDeclarationName(nativeType);
    if (type.isReference) {
      out += `
    if (descriptor.${member.name} != nullptr) {
      Destroy${exportType}(*descriptor.${member.name});
      free((void*) const_cast<${nativeType}*>(descriptor.${member.name}));
    };`;
    }
    else {
      out += `
    Destroy${exportType}(descriptor.${member.name});`;
    }
  }
  else if (type.isStructure && type.isArray) {
    let exportType = getExplortDeclarationName(nativeType);
    if (type.isArrayOfPointers) {
      out += `
    if (descriptor.${type.length} > 0) {
      for (unsigned int ii = 0; ii < descriptor.${type.length}; ++ii) {
        Destroy${exportType}(*descriptor.${member.name}[ii]);
      };
      for (unsigned int ii = 0; ii < descriptor.${type.length}; ++ii) {
        free((void*) const_cast<${nativeType}*>(descriptor.${member.name}[ii]));
      };
      free((void**) const_cast<${nativeType}**>(descriptor.${member.name}));
    }`;
    } else {
      out += `
    if (descriptor.${type.length} > 0) {
      for (unsigned int ii = 0; ii < descriptor.${type.length}; ++ii) {
        Destroy${exportType}(descriptor.${member.name}[ii]);
      };
    }
    if (descriptor.${member.name}) {
      free((void*) const_cast<${nativeType}*>(descriptor.${member.name}));
    }`;
    }
  }
  else if (type.isNumber) {
    // no need to free
  }
  else if (type.isBoolean && !type.isArray) {
    // no need to free
  }
  else if (type.isBoolean && type.isArray) {
    warn(`Unimplemented`);
  }
  else if (type.isEnum && !type.isArray) {
    // no need to free
  }
  else if (type.isEnum && type.isArray) {
    out += `
    if (descriptor.${member.name}) {
      free((void*) const_cast<${nativeType}*>(descriptor.${member.name}));
    }`;
  }
  else if (type.isBitmask && !type.isArray) {
    // no need to free
  }
  else if (type.isBitmask && type.isArray) {
    warn(`Unimplemented`);
  }
  else if (type.isString) {
    out += `
    if (descriptor.${member.name}) {
      delete[] descriptor.${member.name};
    }`;
  }
  return out;
};

function getDecodeStructureParameters(structure, isHeaderFile) {
  let out = ``;
  out += `GPUDevice* device`;
  out += `, const Napi::Value& value`;
  if (isHeaderFile) {
    if (structure.isExtensible) {
      out += `, void* nextInChain = nullptr`;
    }
  } else {
    if (structure.isExtensible) {
      out += `, void* nextInChain`;
    }
  }
  return out;
};

function getDescriptorInstanceReset(structure, opts = DEFAULT_OPTS_RESET_STRUCT) {
  let {padding, output} = opts;
  let {children} = structure;
  let out = ``;
  if (structure.isExtensible) {
    out += `\n${padding}${output.name}.nextInChain = nullptr;`;
  }
  children.map(child => {
    let {type} = child;
    if (type.isReference || type.isObject) {
      out += `\n${padding}${output.name}.${child.name} = nullptr;`;
    }
    if (child.hasOwnProperty("defaultValue")) {
      //console.log("  ", child.name, child.defaultValue, isQuotedString(child.defaultValue));
      if (type.isEnum) {
        out += `\n${padding}${output.name}.${child.name} = static_cast<${type.nativeType}>(${child.defaultValueNative});`;
      } else if (type.isBitmask) {
        out += `\n${padding}${output.name}.${child.name} = static_cast<${type.nativeType}>(${child.defaultValueNative});`;
      } else {
        out += `\n${padding}${output.name}.${child.name} = ${child.defaultValueNative};`;
      }
    }
  });
  return out;
};

export default function(astReference) {
  ast = astReference;
  let {enums, structures} = ast;

  let out = {};
  let vars = {
    enums,
    structures,
    getDestroyStructureMember,
    getDecodeStructureMember,
    getDescriptorInstanceReset,
    getEnumNameFromDawnEnumName,
    getDecodeStructureParameters
  };
  // h
  {
    let template = H_TEMPLATE;
    let output = nunjucks.renderString(template, vars);
    out.header = output;
  }
  // cpp
  {
    let template = CPP_TEMPLATE;
    let output = nunjucks.renderString(template, vars);
    out.source = output;
  }
  return out;
};
