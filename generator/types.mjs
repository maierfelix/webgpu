import {
  warn,
  error,
  getCamelizedName,
  getSnakeCaseName
} from "./utils.mjs";

export function getDawnDeclarationName(name) {
  let camelized = getCamelizedName(name);
  let structName = "Dawn" + (camelized[0].toUpperCase()) + camelized.substr(1);
  return structName;
};

export function getExplortDeclarationName(name) {
  if (name.substr(0, 4) !== "Dawn") warn(`Expected name [0-4] to be 'Dawn'`);
  return "GPU" + name.substr(4);
};

export function getASTNodeByName(name, ast) {
  for (let ii = 0; ii < ast.length; ++ii) {
    let node = ast[ii];
    if (node.textName === name) return node;
  };
  warn(`Cannot resolve AST node for '${name}'`);
  return null;
};

export function getASTCategoryByName(name, ast) {
  let out = getASTNodeByName(name, ast);
  if (!out) warn(`Cannot resolve AST category for '${name}'`);
  return out.category;
};

export function getASTJavaScriptType(type, member, ast) {
  let out = {};
  switch (member.type) {
    // numbers
    case "float":
    case "int32_t":
    case "uint32_t": {
      if (type.isArray) {
        let arrayName = member.type.replace("_t", "");
        arrayName = arrayName[0].toUpperCase() + arrayName.substr(1);
        out.type = arrayName + "Array";
        out.isTypedArray = true;
      } else {
        out.type = "Number";
        out.isNumber = true;
      }
    } break;
    case "int64_t":
    case "uint64_t": {
      if (type.isArray) {
        let arrayName = member.type.replace("_t", "");
        arrayName = arrayName[0].toUpperCase() + arrayName.substr(1);
        out.type = "Big" + arrayName + "Array";
        out.isTypedArray = true;
      } else {
        out.type = "BigInt";
        out.isNumber = true;
        out.isBigInt = true;
      }
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
      if (type.isReference) {
        out.type = "ArrayBuffer";
        out.isArrayBuffer = true;
      } else {
        out.type = "Undefined";
        out.isUndefined = true;
      }
    } break;
    default:
      if (type.isEnum) {
        out.type = "String";
        out.isString = true;
      }
      else if (type.isBitmask) {
        out.type = "Number";
        out.isNumber = true;
      }
      else if (type.isObject) {
        out.type = type.nativeType;
        out.isObject = true;
      }
      else if (type.isStructure) {
        out.type = "Object";
        out.isObject = true;
      }
      else if (type.isFunction) {
        out.type = "Function";
        out.isFunction = true;
      }
      else {
        warn(`Unexpected member type '${member.type}'`);
      }
    break;
  };
  return out;
};

export function getASTType(member, ast) {
  let out = {};
  let type = member.type;
  switch (member.type) {
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
      if (member.annotation !== "const*") {
        warn(`Expected 'annotation' property to be 'const*' for 'char' type`);
      }
      if (!member.hasOwnProperty("length")) {
        warn(`Expected 'length' property to be set for 'char' type`);
      }
      out.isString = true;
      if (member.length === "strlen") out.isDynamicLength = true;
    } break;
    case "void": {
      // TODO
    } break;
    case "error callback":
    case "buffer map read callback":
    case "buffer map write callback":
    case "buffer create mapped callback":
    case "fence on completion callback": {
      out.isFunction = true;
    } break;
    // arbitrary
    default:
      let node = getASTNodeByName(member.type, ast);
      let memberType = getDawnDeclarationName(node.textName);
      // validate category
      {
        switch (node.category) {
          case "enum":
          case "bitmask":
          case "object":
          case "structure":
          case "natively defined": break;
          default: {
            warn(`Unexpected node category '${node.category}'`);
          } break;
        };
      }
      // maps to isEnum, isBitmask etc.
      let key = "is" + node.category[0].toUpperCase() + node.category.substr(1);
      // overwrite default type
      type = memberType;
      out[key] = true;
      out.nativeType = memberType;
    break;
  };
  // validate annotation
  if (member.annotation) {
    switch (member.annotation) {
      case "*":
      case "const*":
      case "const*const*": break;
      default: {
        warn(`Unexpected annotation member '${member.annotation}'`);
      } break;
    };
  }
  if (member.length) {
    if (!Number.isInteger(parseInt(member.length))) {
      out.length = getCamelizedName(member.length);
    }
    else {
      warn(`Expected member-based 'length' but got '${member.length}'`);
    }
  }
  // if the type is generally a reference
  {
    if (member.annotation === "*" || member.annotation === "const*") {
      out.isReference = true;
      if (member.hasOwnProperty("length")) {
        delete out.isNumber;
        delete out.initialValue;
        out.isArray = true;
      }
    }
    else if (member.annotation === "const*const*") {
      out.isArray = true;
      out.isArrayOfPointers = true;
      out.isReference = true;
    }
  }
  if (out.isReference) {
    switch (member.annotation) {
      case "*": out.rawType = `${type}*`; break;
      case "const*": out.rawType = `const ${type}*`; break;
      case "const*const*": out.rawType = `const ${type}* const *`; break;
    };
  } else {
    out.rawType = type;
  }
  if (member.hasOwnProperty("default") || member.optional) out.isOptional = true;
  else out.isRequired = true;
  // append javascript relative type
  out.jsType = getASTJavaScriptType(out, member, ast);
  return out;
};
