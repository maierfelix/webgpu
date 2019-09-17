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
  input: { name: "value" },
  output: { name: "descriptor" }
};

const DEFAULT_OPTS_RESET_STRUCT = {
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
  if (type.isRequired) {
    out += `\n${padding}{`;
  } else {
    out += `\n${padding}if (${input.name}.Has("${member.name}")) {`;
  }
  // decode (unwrap) object typed members
  if (type.isObject && !type.isArray) {
    let unwrapType = getExplortDeclarationName(type.nativeType);
    out += `\n${padding}  if (!(${input.name}.Get("${member.name}").As<Napi::Object>().InstanceOf(${unwrapType}::constructor.Value()))) {`;
    out += `\n${padding}    //NAPI_THROW_JS_CB_ERROR(device, "TypeError", "Expected type '${unwrapType}' for '${structure.externalName}'.'${member.name}'");`;
    out += `\n${padding}    return {};`;
    out += `\n${padding}  }`;
    out += `\n${padding}  ${output.name}.${member.name} = Napi::ObjectWrap<${unwrapType}>::Unwrap(${input.name}.Get("${member.name}").As<Napi::Object>())->instance;`;
  // decode descriptor object array
  } else if (type.isObject && type.isArray) {
    out += `\n${padding}  // UNIMPLEMENTED`;
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
    }
    out += `\n${padding}  Napi::Object $${member.name} = ${input.name}.Get("${member.name}").As<Napi::Object>();`;
    memberTypeStructure.children.map(member => {
      out += getDecodeStructureMember(memberTypeStructure, member, nextOpts);
    });
    // link the struct member reference to top structure
    if (type.isReference) {
      out += `\n${padding}  {`;
      out += `\n${padding}    ${output.name}.${member.name} = new ${type.nativeType};`;
      //out += `\n${padding}    ${getDescriptorInstanceReset(memberTypeStructure)};`;
      out += `\n${padding}    memcpy(const_cast<${type.nativeType}*>(${output.name}.${member.name}), &${member.name}, sizeof(${type.nativeType}));`;
      out += `\n${padding}  }`;
    }
  // decode descriptor structure array
  } else if (type.isStructure && type.isArray) {
    out += `\n${padding}  // UNIMPLEMENTED`;
  // decode numeric typed members
  } else if (type.isNumber) {
    switch (rawType) {
      case "int32_t": {
        out += `\n${padding}  ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().Int32Value();`;
      } break;
      case "uint32_t": {
        out += `\n${padding}  ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().Uint32Value();`;
      } break;
      case "float": {
        out += `\n${padding}  ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().FloatValue();`;
      } break;
      case "uint64_t": {
        out += `\n${padding}  bool lossless;`;
        out += `\n${padding}  ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::BigInt>().Uint64Value(&lossless);`;
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
    out += `\n${padding}  ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Boolean>().Value();`;
  // decode boolean member array
  } else if (type.isBoolean && type.isArray) {
    warn(`Unimplemented Boolean Array member: '${structure.externalName}'.'${member.name}'`);
  // decode enum member
  } else if (type.isEnum && !type.isArray) {
    let decodeMap = getExplortDeclarationName(type.nativeType);
    out += `\n${padding}  ${output.name}.${member.name} = `;
    out += `static_cast<${type.nativeType}>(`;
    out += `${decodeMap}[${input.name}.Get("${member.name}").As<Napi::String>().Utf8Value()]`;
    out += `);`;
  // decode enum member array
  } else if (type.isEnum && type.isArray) {
    out += `\n${padding}  // UNIMPLEMENTED`;
  // decode bitmask member
  } else if (type.isBitmask && !type.isArray) {
    out += `\n${padding}  ${output.name}.${member.name} = static_cast<${type.nativeType}>(${input.name}.Get("${member.name}").As<Napi::Number>().Uint32Value());`;
  // decode bitmask member array
  } else if (type.isBitmask && type.isArray) {
    warn(`Unimplemented Bitmask Array member: '${structure.externalName}'.'${member.name}'`);
  // decode string member
  } else if (type.isString) {
    if (type.isDynamicLength) {
      out += `\n${padding}  ${output.name}.${member.name} = getNAPIStringCopy(${input.name}.Get("${member.name}"));`;
    } else {
      warn(`Cannot handle fixed String length in '${structure.externalName}'.'${member.name}'`);
    }
  } else {
    warn(`Unexpected member type '${rawType}' in '${structure.externalName}'.'${member.name}'`);
  }
  out += `\n${padding}}`;
  return out;
};

function getDecodeStructureParameters(structure) {
  let out = ``;
  out += `Napi::Object& device`;
  out += `, Napi::Object& value`;
  if (structure.isExtensible) {
    out += `, void* nextInChain = nullptr`;
  }
  return out;
};

function getDescriptorInstanceReset(structure, opts = DEFAULT_OPTS_RESET_STRUCT) {
  let {output} = opts;
  let {children} = structure;
  let out = ``;
  if (structure.isExtensible) {
    out += `\n    ${output.name}.nextInChain = nullptr;`;
  }
  //console.log(structure.externalName);
  children.map(child => {
    let {type} = child;
    if (child.hasOwnProperty("defaultValue")) {
      //console.log("  ", child.name, child.defaultValue, isQuotedString(child.defaultValue));
      if (type.isEnum) {
        out += `\n    ${output.name}.${child.name} = static_cast<${type.nativeType}>(${child.defaultValueNative});`;
      } else if (type.isBitmask) {
        out += `\n    ${output.name}.${child.name} = static_cast<${type.nativeType}>(${child.defaultValueNative});`;
      } else {
        out += `\n    ${output.name}.${child.name} = ${child.defaultValueNative};`;
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
