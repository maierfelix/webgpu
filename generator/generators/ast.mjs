import {
  warn,
  getCamelizedName,
  getSnakeCaseName
} from "../utils.mjs";

import {
  getASTType,
  getASTNodeByName,
  getASTCategoryByName,
  getDawnDeclarationName,
  getExplortDeclarationName
} from "../types.mjs";

export default function generateAST(ast) {
  let out = {};
  // normalize
  {
    let normalized = [];
    for (let key in ast) {
      if (!ast.hasOwnProperty(key)) continue;
      if (key === "_comment") continue;
      normalized.push({
        textName: key,
        ...ast[key]
      });
    };
    // overwrite input with normalized input
    ast = normalized;
  }
  // generate enum nodes
  {
    let enums = ast.filter(node => {
      return node.category === "enum";
    });
    enums = enums.map(enu => {
      let node = {};
      let {textName} = enu;
      node.name = getDawnDeclarationName(textName);
      node.externalName = getExplortDeclarationName(node.name);
      node.type = getASTCategoryByName(textName, ast);
      node.textName = textName;
      node.children = [];
      enu.values.map(member => {
        let {value} = member;
        let name = member.name;
        let type = {
          isString: true,
          nativeType: "char",
          jsType: { isString: true, type: "String" }
        };
        let child = {
          name,
          type,
          value
        };
        node.children.push(child);
      });
      return node;
    });
    out.enums = enums;
  }
  // generate bitmask nodes
  {
    let bitmasks = ast.filter(node => {
      return node.category === "bitmask";
    });
    bitmasks = bitmasks.map(bitmask => {
      let node = {};
      let {textName} = bitmask;
      node.name = getDawnDeclarationName(textName);
      node.externalName = getExplortDeclarationName(node.name);
      node.type = getASTCategoryByName(textName, ast);
      node.textName = textName;
      node.children = [];
      bitmask.values.map(member => {
        let {value} = member;
        let name = getSnakeCaseName(member.name);
        let type = {
          isNumber: true,
          nativeType: "uint32_t",
          jsType: { isNumber: true, type: "Number" }
        };
        let child = {
          name,
          type,
          value
        };
        node.children.push(child);
      });
      return node;
    });
    out.bitmasks = bitmasks;
  }
  // generate object nodes
  {
    let objects = ast.filter(node => {
      return node.category === "object";
    });
    objects = objects.map(object => {
      let node = {};
      let {textName} = object;
      node.name = getDawnDeclarationName(textName);
      node.externalName = getExplortDeclarationName(node.name);
      node.type = getASTCategoryByName(textName, ast);
      node.textName = textName;
      node.children = [];
      // process the object's methods
      (object.methods || []).map(method => {
        let name = getCamelizedName(method.name);
        let child = {
          name,
          children: []
        };
        if (method.returns) {
          child.type = getASTType({ type: method.returns }, ast);
        } else {
          child.type = getASTType({ type: "void" }, ast);
        }
        // process the method's arguments
        (method.args || []).map(arg => {
          let name = getCamelizedName(arg.name);
          let type = getASTType(arg, ast);
          let argChild = {
            name,
            type
          };
          if (arg.optional) argChild.isOptional = true;
          child.children.push(argChild);
        });
        node.children.push(child);
      });
      return node;
    });
    out.objects = objects;
  }
  // generate structure nodes
  {
    let structures = ast.filter(node => {
      return node.category === "structure";
    });
    structures = structures.map(structure => {
      let node = {};
      let {textName, extensible} = structure;
      node.name = getDawnDeclarationName(textName);
      node.externalName = getExplortDeclarationName(node.name);
      node.type = getASTCategoryByName(textName, ast);
      if (extensible) node.isExtensible = true;
      node.textName = textName;
      node.children = [];
      structure.members.map(member => {
        let name = getCamelizedName(member.name);
        let type = getASTType(member, ast);
        let child = {
          name,
          type
        };
        if (member.optional) child.isOptional = true;
        if (member.hasOwnProperty("default") && member.default !== "undefined") {
          let defaultValue = member.default;
          if (defaultValue === "true" || defaultValue === "false") {
            child.defaultValue = defaultValue === "true";
            child.defaultValueNative = defaultValue;
          } else if (Number.isInteger(parseInt(defaultValue))) {
            child.defaultValue = String(defaultValue);
            child.defaultValueNative = defaultValue;
          } else if (typeof defaultValue === "string") {
            child.defaultValue = `"${member.default}"`;
            child.defaultValueNative = getASTNodeByName(member.type, ast).values.filter(({ name }) => {
              return name === member.default;
            })[0].value;
          } else {
            warn(`Unexpected default value for '${node.externalName}'.'${child.name}'`);
          }
        }
        node.children.push(child);
      });
      return node;
    });
    out.structures = structures;
  }
  return out;
};
