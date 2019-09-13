import fs from "fs";

export function warn() {
  let args = [];
  for (let ii = 0; ii < arguments.length; ++ii) args.push(arguments[ii]);
  let str = args.join(", ");
  console.log(`\x1b[33m%s\x1b[0m`, `Warning: ${str}`);
};

export function error() {
  let args = [];
  for (let ii = 0; ii < arguments.length; ++ii) args.push(arguments[ii]);
  let str = args.join(", ");
  process.stderr.write(`\x1b[31mError: ${str}\n\x1b[0m`);
};

export function getPlatform() {
  let fakePlatform = process.env.npm_config_fake_platform;
  if (fakePlatform) {
    switch (fakePlatform) {
      case "win32":
      case "linux":
      case "darwin":
        break;
      default:
        throw new Error(`Invalid fake platform! Aborting..`);
    };
    return fakePlatform;
  }
  return process.platform;
};

export function getCamelizedName(name) {
  return (
    name
    .replace(/\s(.)/g, function($1) { return $1.toUpperCase(); })
    .replace(/\s/g, '')
    .replace(/^(.)/, function($1) { return $1.toLowerCase(); })
  );
};

export function getNativeStructureName(name) {
  let camelized = getCamelizedName(name);
  let structName = "Dawn" + (camelized[0].toUpperCase()) + camelized.substr(1);
  return structName;
};

export function getASTCategoryByName(name, ast) {
  for (let ii = 0; ii < ast.length; ++ii) {
    let node = ast[ii];
    if (node.textName === name) return node.category;
  };
  warn(`Cannot resolve AST category for '${name}'`);
  return null;
};

export function getASTJavaScriptType(type, member, ast) {
  let out = {};
  switch (member.type) {
    // numbers
    case "float":
    case "int32_t":
    case "uint32_t": {
      out.type = "Number";
      out.isNumber = true;
    } break;
    case "uint64_t": {
      out.type = "BigInt";
      out.isNumber = true;
      out.isBigInt = true;
    } break;
    case "bool": {
      out.type = "Boolean";
      out.isBoolean = true;
    } break;
    case "char": {
      out.type = "String";
      out.isString = true;
    } break;
    case "void": {
      out.type = "ArrayBuffer";
      out.isArrayBuffer = true;
    } break;
  };
  return out;
};

export function getASTType(member, ast) {
  let out = {};
  let {type} = member;
  switch (type) {
    // numbers
    case "float":
    case "int32_t":
    case "uint32_t":
    case "uint64_t": {
      let initialValue = member.default + "";
      if (!member.hasOwnProperty("default")) {
        initialValue = "0";
      }
      out.isNumber = true;
      out.initialValue = initialValue;
    } break;
    case "bool": {
      let initialValue = member.default + "";
      if (!member.hasOwnProperty("default")) {
        initialValue = "false";
      }
      out.isBoolean = true;
      out.initialValue = initialValue;
    } break;
    case "char": {
      let {length} = member;
      if (member.annotation !== "const*") {
        warn(`Expected 'annotation' property to be 'const*' for 'char' type`);
      }
      if (!member.hasOwnProperty("length")) {
        warn(`Expected 'length' property to be set for 'char' type`);
      }
      out.isString = true;
      if (length === "strlen") out.isDynamicLength = true;
      if (!out.isDynamicLength) out.length = length;
    } break;
    case "void": {
      // TODO
    } break;
  };
  if (member.annotation === "const*") out.isReference = true;
  if (out.isReference) {
    out.rawType = `const ${type}*`;
  } else {
    out.rawType = type;
  }
  // append javascript relative type
  out.jsType = getASTJavaScriptType(out, member, ast);
  return out;
};
