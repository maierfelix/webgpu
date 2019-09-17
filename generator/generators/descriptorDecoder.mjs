import fs from "fs";
import nunjucks from "nunjucks";

import pkg from "../../package.json";

import {
  warn,
  isQuotedString
} from "../utils.mjs";

import {
  getExplortDeclarationName
} from "../types.mjs";

let ast = null;

const DEFAULT_OPTS_DECODE_STRUCT_MEMBER = {
  input: { name: "obj" },
  output: { name: "descriptor" }
};

const H_TEMPLATE = fs.readFileSync(`${pkg.config.TEMPLATE_DIR}/DescriptorDecoder-h.njk`, "utf-8");
const CPP_TEMPLATE = fs.readFileSync(`${pkg.config.TEMPLATE_DIR}/DescriptorDecoder-cpp.njk`, "utf-8");

nunjucks.configure({ autoescape: true });

export function getDecodeStructureMember(structure, member, opts = DEFAULT_OPTS_DECODE_STRUCT_MEMBER) {
  let {type} = member;
  let {jsType, rawType} = type;
  let {input, output} = opts;
  let out = ``;
  if (type.isRequired) {
    out += `\n    {`;
  } else {
    out += `\n    if (${input.name}.Has("${member.name}")) {`;
  }
  // decode (unwrap) object typed members
  if (type.isObject) {

  // decode numeric typed members
  } else if (type.isNumber) {
    switch (rawType) {
      case "int32_t": {
        out += `\n      ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().Int32Value();`;
      } break;
      case "uint32_t": {
        out += `\n      ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().Uint32Value();`;
      } break;
      case "float": {
        out += `\n      ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Number>().FloatValue();`;
      } break;
      case "uint64_t": {
        out += `\n      bool lossless;`;
        out += `\n      ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::BigInt>().Uint64Value(&lossless);`;
      } break;
      case "const uint32_t*": {
        out += `\n      size_t size;`;
        out += `\n      ${output.name}.${member.name} = getTypedArrayData<uint32_t>(${input.name}.Get("${member.name}"), &size);`;
        out += `\n      ${output.name}.${type.length} = static_cast<uint32_t>(size);`;
      } break;
      default: {
        warn(`Unexpected member type '${rawType}' in '${structure.externalName}'.'${member.name}'`);
      } break;
    };
  } else if (type.isBoolean) {
    out += `\n      ${output.name}.${member.name} = ${input.name}.Get("${member.name}").As<Napi::Boolean>().Value();`;
  } else if (type.isEnum) {
    let decodeMap = getExplortDeclarationName(type.nativeType);
    out += `\n      ${output.name}.${member.name} = `;
    out += `static_cast<${type.nativeType}>(`;
    out += `${decodeMap}[${input.name}.Get("${member.name}").As<Napi::String>().Utf8Value()]`;
    out += `);`;
  } else if (type.isBitmask) {
    out += `\n      ${output.name}.${member.name} = static_cast<${type.nativeType}>(${input.name}.Get("${member.name}").As<Napi::Number>().Uint32Value());`;
  } else if (type.isString) {
    if (type.isDynamicLength) {
      out += `\n      ${output.name}.${member.name} = getNAPIStringCopy(${input.name}.Get("${member.name}"));`;
    } else {
      warn(`Cannot handle fixed String length in '${structure.externalName}'.'${member.name}'`);
    }
  } else {
    //console.log(member);
  }
  out += `\n    }`;
  return out;
};

function getDecodeStructureParameters(structure) {
  let out = ``;
  out += `Napi::Value& value`;
  if (structure.isExtensible) {
    out += `, void* nextInChain = nullptr`;
  }
  return out;
};

function getDescriptorInstanceReset(structure) {
  let {children} = structure;
  let out = ``;
  if (structure.isExtensible) {
    out += `\n    descriptor.nextInChain = nullptr;`;
  }
  //console.log(structure.externalName);
  children.map(child => {
    if (child.hasOwnProperty("defaultValue")) {
      //console.log("  ", child.name, child.defaultValue, isQuotedString(child.defaultValue));
      out += `\n    descriptor.${child.name} = ${child.defaultValueNative};`;
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
