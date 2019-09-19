import fs from "fs";
import nunjucks from "nunjucks";

import pkg from "../../package.json";

import {
  warn,
  isQuotedString
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

export function getDecodeStructureMember(structure, member, opts = DEFAULT_OPTS_DECODE_STRUCT_MEMBER) {
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
  // decode (unwrap) object typed members
  if (type.isObject && !type.isArray) {
    let unwrapType = getExplortDeclarationName(type.nativeType);
    out += `\n${padding}if (!(${input.name}.Get("${member.name}").As<Napi::Object>().InstanceOf(${unwrapType}::constructor.Value()))) {`;
    out += `\n${padding}  Napi::String type = Napi::String::New(value.Env(), "Type");`;
    out += `\n${padding}  Napi::String message = Napi::String::New(value.Env(), "Expected type '${unwrapType}' for '${structure.externalName}'.'${member.name}'");`;
    out += `\n${padding}  device->throwCallbackError(type, message);`;
    out += `\n${padding}  return {};`;
    out += `\n${padding}}`;
    out += `\n${padding}${output.name}.${member.name} = Napi::ObjectWrap<${unwrapType}>::Unwrap(${input.name}.Get("${member.name}").As<Napi::Object>())->instance;`;
  // decode descriptor object array
  } else if (type.isObject && type.isArray) {
    let unwrapType = getExplortDeclarationName(type.nativeType);
    out += `
${padding}Napi::Array array = ${input.name}.Get("${member.name}").As<Napi::Array>();
${padding}uint32_t length = array.Length();
${padding}std::vector<${type.nativeType}> data;
${padding}for (unsigned int ii = 0; ii < length; ++ii) {
${padding}  Napi::Object item = array.Get(ii).As<Napi::Object>();
${padding}  ${type.nativeType} value = Napi::ObjectWrap<${unwrapType}>::Unwrap(item)->instance;
${padding}  data.push_back(value);
${padding}};`;
    out += `
${padding}${output.name}.${type.length} = length;
${padding}${output.name}.${member.name} = data.data();`;
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
      out += getDecodeStructureMember(memberTypeStructure, member, nextOpts);
    });
    // link the struct member reference to top structure
    if (type.isReference) {
      out += `\n${padding}{`;
      out += `\n${padding}  ${output.name}.${member.name} = new ${type.nativeType};`;
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
${padding}uint32_t length = array.Length();
${padding}std::vector<${type.nativeType}> data;
${padding}for (unsigned int ii = 0; ii < length; ++ii) {
${padding}  Napi::Object item = array.Get(ii).As<Napi::Object>();
${padding}  ${type.nativeType} $${member.name};`;

    // reset descriptor
    let resetOpts = {
      padding: padding + "  ",
      output: { name: "$" + member.name }
    };
    out += getDescriptorInstanceReset(memberTypeStructure, resetOpts);

    let nextOpts = {
      padding: opts.padding + `    `,
      input: { name: `item` },
      output: { name: `$${member.name}` }
    };
    memberTypeStructure.children.map(member => {
      out += getDecodeStructureMember(memberTypeStructure, member, nextOpts);
    });

    out += `
${padding}  data.push_back($${member.name});
${padding}};`;
    // create array of pointers
    if (type.isArrayOfPointers) {
out += `
${padding}${output.name}.${type.length} = length;
${padding}std::vector<${type.nativeType}*> dst(length);
${padding}std::transform(data.begin(), data.end(), dst.begin(), [](${type.nativeType}& d) { return &d; });
${padding}${output.name}.${member.name} = dst.data();`;
    // normal struct array
    } else {
out += `
${padding}${output.name}.${type.length} = length;
${padding}${output.name}.${member.name} = data.data();`;
    }
  // decode numeric typed members
  } else if (type.isNumber) {
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
        out += `\n${padding}  bool lossless;`;
        out += `\n${padding}  ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::BigInt>().Uint64Value(&lossless);`;
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
    out += `${decodeMap}[${input.name}.Get("${member.name}").As<Napi::String>().Utf8Value()]`;
    out += `);`;
  // decode enum member array
  } else if (type.isEnum && type.isArray) {
    let decodeMap = getExplortDeclarationName(type.nativeType);
    out += `
${padding}Napi::Array array = ${input.name}.Get("${member.name}").As<Napi::Array>();
${padding}uint32_t length = array.Length();
${padding}std::vector<${type.nativeType}> data;
${padding}for (unsigned int ii = 0; ii < length; ++ii) {
${padding}  Napi::Object item = array.Get(ii).As<Napi::Object>();
${padding}  ${type.nativeType} value = static_cast<${type.nativeType}>(
${padding}    ${decodeMap}[item.As<Napi::String>().Utf8Value()]
${padding}  );
${padding}  data.push_back(value);
${padding}};`;
    out += `
${padding}${output.name}.${member.name} = data.data();`;
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
  } else {
    warn(`Unexpected member type '${rawType}' in '${structure.externalName}'.'${member.name}'`);
  }
  if (type.isOptional || type.isArray) {
    padding = opts.padding = opts.padding.substr(0, opts.padding.length - 2);
    out += `\n${padding}}`;
  }
  return out;
};

function getDecodeStructureParameters(structure, isHeaderFile) {
  let out = ``;
  out += `GPUDevice* device`;
  out += `, Napi::Value& value`;
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
  //console.log(structure.externalName);
  children.map(child => {
    let {type} = child;
    if (type.isReference) {
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
    getDecodeStructureMember,
    getDescriptorInstanceReset,
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
